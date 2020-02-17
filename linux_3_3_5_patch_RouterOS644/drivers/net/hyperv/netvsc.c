/*
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <asm/sync_bitops.h>

#include "hyperv_net.h"


static struct netvsc_device *alloc_net_device(void)
{
	struct netvsc_device *net_device;

	net_device = kzalloc(sizeof(struct netvsc_device), GFP_KERNEL);
	if (!net_device)
		return NULL;

	net_device->recv_section_size = NETVSC_RECV_SECTION_SIZE;
	net_device->send_section_size = NETVSC_SEND_SECTION_SIZE;

	init_waitqueue_head(&net_device->wait_drain);
	net_device->destroy = false;

	return net_device;
}

static void free_netvsc_device(struct netvsc_device *net_device)
{

	vfree(net_device->recv_buf);
	if (net_device->send_buf) {
		vfree(net_device->send_buf);
	}
	kfree(net_device->send_section_map);

	kfree(net_device);
}

static struct netvsc_device *get_outbound_net_device(struct hv_device *device)
{
	struct netvsc_device *net_device = hv_device_to_netvsc_device(device);

	if (net_device && net_device->destroy)
		net_device = NULL;

	return net_device;
}

static struct netvsc_device *get_inbound_net_device(struct hv_device *device)
{
	struct netvsc_device *net_device = hv_device_to_netvsc_device(device);

	if (!net_device)
		goto get_in_err;

	if (net_device->destroy &&
		atomic_read(&net_device->num_outstanding_sends) == 0)
		net_device = NULL;

get_in_err:
	return net_device;
}

static void netvsc_revoke_recv_buf(struct hv_device *device,
				   struct netvsc_device *net_device,
				   struct net_device *ndev)
{
	struct nvsp_message *revoke_packet;
	int ret;

	/*
	 * If we got a section count, it means we received a
	 * SendReceiveBufferComplete msg (ie sent
	 * NvspMessage1TypeSendReceiveBuffer msg) therefore, we need
	 * to send a revoke msg here
	 */
	if (net_device->recv_section_cnt) {
		/* Send the revoke receive buffer */
		revoke_packet = &net_device->revoke_packet;
		memset(revoke_packet, 0, sizeof(struct nvsp_message));

		revoke_packet->hdr.msg_type =
			NVSP_MSG1_TYPE_REVOKE_RECV_BUF;
		revoke_packet->msg.v1_msg.
		revoke_recv_buf.id = NETVSC_RECEIVE_BUFFER_ID;

		ret = vmbus_sendpacket(device->channel,
				       revoke_packet,
				       sizeof(struct nvsp_message),
				       (unsigned long)revoke_packet,
				       VM_PKT_DATA_INBAND, 0);
		/*
		 * If we failed here, we might as well return and
		 * have a leak rather than continue and a bugchk
		 */
		if (ret != 0) {
			netdev_err(ndev, "unable to send "
				"revoke receive buffer to netvsp\n");
			return;
		}
		net_device->recv_section_cnt = 0;
	}
}

static void netvsc_revoke_send_buf(struct hv_device *device,
				   struct netvsc_device *net_device,
				   struct net_device *ndev)
{
	struct nvsp_message *revoke_packet;
	int ret;

	/* Deal with the send buffer we may have setup.
	 * If we got a  send section size, it means we received a
	 * SendsendBufferComplete msg (ie sent
	 * NvspMessage1TypeSendReceiveBuffer msg) therefore, we need
	 * to send a revoke msg here
	 */
	if (net_device->send_section_cnt) {
		/* Send the revoke receive buffer */
		revoke_packet = &net_device->revoke_packet;
		memset(revoke_packet, 0, sizeof(struct nvsp_message));

		revoke_packet->hdr.msg_type =
			NVSP_MSG1_TYPE_REVOKE_SEND_BUF;
		revoke_packet->msg.v1_msg.revoke_recv_buf.id = 0;

		ret = vmbus_sendpacket(device->channel,
				       revoke_packet,
				       sizeof(struct nvsp_message),
				       (unsigned long)revoke_packet,
				       VM_PKT_DATA_INBAND, 0);
		/* If we failed here, we might as well return and
		 * have a leak rather than continue and a bugchk
		 */
		if (ret != 0) {
			netdev_err(ndev, "unable to send "
				   "revoke send buffer to netvsp\n");
			return;
		}
		net_device->send_section_cnt = 0;
	}
}

static void netvsc_teardown_recv_gpadl(struct hv_device *device,
				       struct netvsc_device *net_device,
				       struct net_device *ndev)
{
	int ret;

	/* Teardown the gpadl on the vsp end */
	if (net_device->recv_buf_gpadl_handle) {
		ret = vmbus_teardown_gpadl(device->channel,
			   net_device->recv_buf_gpadl_handle);

		/* If we failed here, we might as well return and have a leak
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			netdev_err(ndev,
				   "unable to teardown receive buffer's gpadl\n");
			return;
		}
		net_device->recv_buf_gpadl_handle = 0;
	}
}

static void netvsc_teardown_send_gpadl(struct hv_device *device,
				       struct netvsc_device *net_device,
				       struct net_device *ndev)
{
	int ret;

	if (net_device->send_buf_gpadl_handle) {
		ret = vmbus_teardown_gpadl(device->channel,
					   net_device->send_buf_gpadl_handle);

		/* If we failed here, we might as well return and have a leak
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			netdev_err(ndev,
				   "unable to teardown send buffer's gpadl\n");
			return;
		}
		net_device->send_buf_gpadl_handle = 0;
	}
}

static int netvsc_init_buf(struct hv_device *device,
			   struct netvsc_device *net_device,
			   const struct netvsc_device_info *device_info)
{
	struct nvsp_1_message_send_receive_buffer_complete *resp;
	struct net_device *ndev = hv_get_drvdata(device);
	struct nvsp_message *init_packet;
	unsigned int buf_size;
	int ret = 0;

	/* Get receive buffer area. */
	buf_size = device_info->recv_sections * net_device->recv_section_size;
	buf_size = roundup(buf_size, PAGE_SIZE);

	net_device->recv_buf = vzalloc(buf_size);
	if (!net_device->recv_buf) {
		netdev_err(ndev,
			   "unable to allocate receive buffer of size %u\n",
			   buf_size);
		ret = -ENOMEM;
		goto cleanup;
	}

	/*
	 * Establish the gpadl handle for this buffer on this
	 * channel.  Note: This call uses the vmbus connection rather
	 * than the channel to establish the gpadl handle.
	 */
	ret = vmbus_establish_gpadl(device->channel, net_device->recv_buf,
				    buf_size,
				    &net_device->recv_buf_gpadl_handle);
	if (ret != 0) {
		netdev_err(ndev,
			"unable to establish receive buffer's gpadl\n");
		goto cleanup;
	}


	/* Notify the NetVsp of the gpadl handle */
	init_packet = &net_device->channel_init_pkt;

	memset(init_packet, 0, sizeof(struct nvsp_message));

	init_packet->hdr.msg_type = NVSP_MSG1_TYPE_SEND_RECV_BUF;
	init_packet->msg.v1_msg.send_recv_buf.
		gpadl_handle = net_device->recv_buf_gpadl_handle;
	init_packet->msg.v1_msg.
		send_recv_buf.id = NETVSC_RECEIVE_BUFFER_ID;

	/* Send the gpadl notification request */
	ret = vmbus_sendpacket(device->channel, init_packet,
			       sizeof(struct nvsp_message),
			       (unsigned long)init_packet,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		netdev_err(ndev,
			"unable to send receive buffer's gpadl to netvsp\n");
		goto cleanup;
	}

	wait_for_completion(&net_device->channel_init_wait);

	/* Check the response */
	resp = &init_packet->msg.v1_msg.send_recv_buf_complete;
	if (resp->status != NVSP_STAT_SUCCESS) {
		netdev_err(ndev,
			   "Unable to complete receive buffer initialization with NetVsp - status %d\n",
			   resp->status);
		ret = -EINVAL;
		goto cleanup;
	}

	/* Parse the response */
	netdev_dbg(ndev, "Receive sections: %u sub_allocs: size %u count: %u\n",
		   resp->num_sections, resp->sections[0].sub_alloc_size,
		   resp->sections[0].num_sub_allocs);

	/* There should only be one section for the entire receive buffer */
	if (resp->num_sections != 1 || resp->sections[0].offset != 0) {
		ret = -EINVAL;
		goto cleanup;
	}

	net_device->recv_section_size = resp->sections[0].sub_alloc_size;
	net_device->recv_section_cnt = resp->sections[0].num_sub_allocs;

	/* Now setup the send buffer.
	 */
	buf_size = device_info->send_sections * net_device->send_section_size;
	buf_size = round_up(buf_size, PAGE_SIZE);

	net_device->send_buf = vzalloc(buf_size);
	if (!net_device->send_buf) {
		netdev_err(ndev, "unable to allocate send buffer of size %u\n",
			   buf_size);
		ret = -ENOMEM;
		goto cleanup;
	}

	/* Establish the gpadl handle for this buffer on this
	 * channel.  Note: This call uses the vmbus connection rather
	 * than the channel to establish the gpadl handle.
	 */
	ret = vmbus_establish_gpadl(device->channel, net_device->send_buf,
				    buf_size,
				    &net_device->send_buf_gpadl_handle);
	if (ret != 0) {
		netdev_err(ndev,
			   "unable to establish send buffer's gpadl\n");
		goto cleanup;
	}

	/* Notify the NetVsp of the gpadl handle */
	init_packet = &net_device->channel_init_pkt;
	memset(init_packet, 0, sizeof(struct nvsp_message));
	init_packet->hdr.msg_type = NVSP_MSG1_TYPE_SEND_SEND_BUF;
	init_packet->msg.v1_msg.send_recv_buf.gpadl_handle =
		net_device->send_buf_gpadl_handle;
	init_packet->msg.v1_msg.send_recv_buf.id = 0;

	/* Send the gpadl notification request */
	ret = vmbus_sendpacket(device->channel, init_packet,
			       sizeof(struct nvsp_message),
			       (unsigned long)init_packet,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		netdev_err(ndev,
			   "unable to send send buffer's gpadl to netvsp\n");
		goto cleanup;
	}

	wait_for_completion(&net_device->channel_init_wait);

	/* Check the response */
	if (init_packet->msg.v1_msg.
	    send_send_buf_complete.status != NVSP_STAT_SUCCESS) {
		netdev_err(ndev, "Unable to complete send buffer "
			   "initialization with NetVsp - status %d\n",
			   init_packet->msg.v1_msg.
			   send_recv_buf_complete.status);
		ret = -EINVAL;
		goto cleanup;
	}

	/* Parse the response */
	net_device->send_section_size = init_packet->msg.
				v1_msg.send_send_buf_complete.section_size;


	/* Section count is simply the size divided by the section size. */
	net_device->send_section_cnt = buf_size / net_device->send_section_size;

	dev_info(&device->device, "Send section size: %d, Section count:%d\n",
		 net_device->send_section_size, net_device->send_section_cnt);

	/* Setup state for managing the send buffer. */
	net_device->map_words = DIV_ROUND_UP(net_device->send_section_cnt,
					     BITS_PER_LONG);

	net_device->send_section_map =
		kzalloc(net_device->map_words * sizeof(ulong), GFP_KERNEL);
	if (net_device->send_section_map == NULL)
		goto cleanup;

	goto exit;

cleanup:
	netvsc_revoke_recv_buf(device, net_device, ndev);
	netvsc_teardown_recv_gpadl(device, net_device, ndev);
	netvsc_revoke_send_buf(device, net_device, ndev);
	netvsc_teardown_send_gpadl(device, net_device, ndev);

exit:
	return ret;
}


/* Negotiate NVSP protocol version */
static int negotiate_nvsp_ver(struct hv_device *device,
			      struct netvsc_device *net_device,
			      struct nvsp_message *init_packet,
			      u32 nvsp_ver)
{
	struct net_device *ndev = hv_get_drvdata(device);
	int ret;

	memset(init_packet, 0, sizeof(struct nvsp_message));
	init_packet->hdr.msg_type = NVSP_MSG_TYPE_INIT;
	init_packet->msg.init_msg.init.min_protocol_ver = nvsp_ver;
	init_packet->msg.init_msg.init.max_protocol_ver = nvsp_ver;

	/* Send the init request */
	ret = vmbus_sendpacket(device->channel, init_packet,
			       sizeof(struct nvsp_message),
			       (unsigned long)init_packet,
			       VM_PKT_DATA_INBAND,
			       VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0)
		return ret;

	wait_for_completion(&net_device->channel_init_wait);

	if (init_packet->msg.init_msg.init_complete.status !=
	    NVSP_STAT_SUCCESS)
		return -EINVAL;

	if (nvsp_ver == NVSP_PROTOCOL_VERSION_1)
		return 0;

	/* NVSPv2 only: Send NDIS config */
	memset(init_packet, 0, sizeof(struct nvsp_message));
	init_packet->hdr.msg_type = NVSP_MSG2_TYPE_SEND_NDIS_CONFIG;
	init_packet->msg.v2_msg.send_ndis_config.mtu = ndev->mtu;
	init_packet->msg.v2_msg.send_ndis_config.capability.ieee8021q = 1;

	ret = vmbus_sendpacket(device->channel, init_packet,
				sizeof(struct nvsp_message),
				(unsigned long)init_packet,
				VM_PKT_DATA_INBAND, 0);

	return ret;
}

static int netvsc_connect_vsp(struct hv_device *device,
			      struct netvsc_device *net_device,
			      const struct netvsc_device_info *device_info)
{
	int ret;
	struct nvsp_message *init_packet;
	int ndis_version;

	u32 ver_list[] = { NVSP_PROTOCOL_VERSION_1, NVSP_PROTOCOL_VERSION_2,
		NVSP_PROTOCOL_VERSION_4, NVSP_PROTOCOL_VERSION_5 };
	int i, num_ver = 4; /* number of different NVSP versions */

	init_packet = &net_device->channel_init_pkt;

	/* Negotiate the latest NVSP protocol supported */
	for (i = num_ver - 1; i >= 0; i--)
	if (negotiate_nvsp_ver(device, net_device, init_packet,
				       ver_list[i])  == 0) {
			net_device->nvsp_version = ver_list[i];
			break;
		}

	if (i < 0) {
		ret = -EPROTO;
		goto cleanup;
	}

	pr_debug("Negotiated NVSP version:%x\n", net_device->nvsp_version);

	/* Send the ndis version */
	memset(init_packet, 0, sizeof(struct nvsp_message));

	if (net_device->nvsp_version <= NVSP_PROTOCOL_VERSION_4)
		ndis_version = 0x00060001;
	else
		ndis_version = 0x0006001e;

	init_packet->hdr.msg_type = NVSP_MSG1_TYPE_SEND_NDIS_VER;
	init_packet->msg.v1_msg.
		send_ndis_ver.ndis_major_ver =
				(ndis_version & 0xFFFF0000) >> 16;
	init_packet->msg.v1_msg.
		send_ndis_ver.ndis_minor_ver =
				ndis_version & 0xFFFF;

	/* Send the init request */
	ret = vmbus_sendpacket(device->channel, init_packet,
				sizeof(struct nvsp_message),
				(unsigned long)init_packet,
				VM_PKT_DATA_INBAND, 0);
	if (ret != 0)
		goto cleanup;

	ret = netvsc_init_buf(device, net_device, device_info);

cleanup:
	return ret;
}

/*
 * netvsc_device_remove - Callback when the root bus device is removed
 */
int netvsc_device_remove(struct hv_device *device)
{
	struct net_device *ndev = hv_get_drvdata(device);
	struct net_device_context *net_device_ctx = netdev_priv(ndev);
	struct netvsc_device *net_device = net_device_ctx->nvdev;
	unsigned long flags;

	net_device_ctx->nvdev = NULL;

	/*
	 * Revoke receive buffer. If host is pre-Win2016 then tear down
	 * receive buffer GPADL. Do the same for send buffer.
	 */
	netvsc_revoke_recv_buf(device, net_device, ndev);
	if (vmbus_proto_version < VERSION_WIN10)
		netvsc_teardown_recv_gpadl(device, net_device, ndev);

	netvsc_revoke_send_buf(device, net_device, ndev);
	if (vmbus_proto_version < VERSION_WIN10)
		netvsc_teardown_send_gpadl(device, net_device, ndev);

	/*
	 * At this point, no one should be accessing net_device
	 * except in here
	 */
	dev_notice(&device->device, "net device safe to remove\n");

	/* Now, we can close the channel safely */
	vmbus_close(device->channel);

	/*
	 * If host is Win2016 or higher then we do the GPADL tear down
	 * here after VMBus is closed.
	*/
	if (vmbus_proto_version >= VERSION_WIN10) {
		netvsc_teardown_recv_gpadl(device, net_device, ndev);
		netvsc_teardown_send_gpadl(device, net_device, ndev);
	}

	/* Release all resources */
	if (net_device->sub_cb_buf)
		vfree(net_device->sub_cb_buf);

	free_netvsc_device(net_device);
	return 0;
}


#define RING_AVAIL_PERCENT_HIWATER 20
#define RING_AVAIL_PERCENT_LOWATER 10

/*
 * Get the percentage of available bytes to write in the ring.
 * The return value is in range from 0 to 100.
 */
static inline u32 hv_ringbuf_avail_percent(
		struct hv_ring_buffer_info *ring_info)
{
	u32 avail_read, avail_write;

	hv_get_ringbuffer_availbytes(ring_info, &avail_read, &avail_write);

	return avail_write * 100 / ring_info->ring_datasize;
}

static inline void netvsc_free_send_slot(struct netvsc_device *net_device,
					 u32 index)
{
	sync_change_bit(index, net_device->send_section_map);
}

static void netvsc_send_completion(struct netvsc_device *net_device,
				   struct hv_device *device,
				   struct vmpacket_descriptor *packet)
{
	struct nvsp_message *nvsp_packet;
	struct hv_netvsc_packet *nvsc_packet;
	struct net_device *ndev = hv_get_drvdata(device);
	struct net_device_context *net_device_ctx = netdev_priv(ndev);
	u32 send_index;


	nvsp_packet = (struct nvsp_message *)((unsigned long)packet +
			(packet->offset8 << 3));

	if ((nvsp_packet->hdr.msg_type == NVSP_MSG_TYPE_INIT_COMPLETE) ||
	    (nvsp_packet->hdr.msg_type ==
	     NVSP_MSG1_TYPE_SEND_RECV_BUF_COMPLETE) ||
	    (nvsp_packet->hdr.msg_type ==
	     NVSP_MSG1_TYPE_SEND_SEND_BUF_COMPLETE) ||
	    (nvsp_packet->hdr.msg_type ==
	     NVSP_MSG5_TYPE_SUBCHANNEL)) {
		/* Copy the response back */
		memcpy(&net_device->channel_init_pkt, nvsp_packet,
		       sizeof(struct nvsp_message));
		complete(&net_device->channel_init_wait);
	} else if (nvsp_packet->hdr.msg_type ==
		   NVSP_MSG1_TYPE_SEND_RNDIS_PKT_COMPLETE) {
		int num_outstanding_sends;
		u16 q_idx = 0;
		struct vmbus_channel *channel = device->channel;
		int queue_sends;

		/* Get the send context */
		nvsc_packet = (struct hv_netvsc_packet *)(unsigned long)
			packet->trans_id;

		/* Notify the layer above us */
		if (nvsc_packet) {
			send_index = nvsc_packet->send_buf_index;
			if (send_index != NETVSC_INVALID_INDEX)
				netvsc_free_send_slot(net_device, send_index);
			q_idx = nvsc_packet->q_idx;
			channel = nvsc_packet->channel;
			nvsc_packet->send_completion(nvsc_packet->
						     send_completion_ctx);
		}

		num_outstanding_sends =
			atomic_dec_return(&net_device->num_outstanding_sends);
		queue_sends = atomic_dec_return(&net_device->
						queue_sends[q_idx]);

		if (net_device->destroy && num_outstanding_sends == 0)
			wake_up(&net_device->wait_drain);

		if (netif_tx_queue_stopped(netdev_get_tx_queue(ndev, q_idx)) &&
		    !net_device_ctx->start_remove &&
		    (hv_ringbuf_avail_percent(&channel->outbound) >
		     RING_AVAIL_PERCENT_HIWATER || queue_sends < 1))
				netif_tx_wake_queue(netdev_get_tx_queue(
						    ndev, q_idx));
	} else {
		netdev_err(ndev, "Unknown send completion packet type- "
			   "%d received!!\n", nvsp_packet->hdr.msg_type);
	}

}

static u32 netvsc_get_next_send_section(struct netvsc_device *net_device)
{
	unsigned long index;
	u32 max_words = net_device->map_words;
	unsigned long *map_addr = (unsigned long *)net_device->send_section_map;
	u32 section_cnt = net_device->send_section_cnt;
	int ret_val = NETVSC_INVALID_INDEX;
	int i;
	int prev_val;

	for (i = 0; i < max_words; i++) {
		if (!~(map_addr[i]))
			continue;
		index = ffz(map_addr[i]);
		prev_val = sync_test_and_set_bit(index, &map_addr[i]);
		if (prev_val)
			continue;
		if ((index + (i * BITS_PER_LONG)) >= section_cnt)
			break;
		ret_val = (index + (i * BITS_PER_LONG));
		break;
	}
	return ret_val;
}

u32 netvsc_copy_to_send_buf(struct netvsc_device *net_device,
			    unsigned int section_index,
			    struct hv_netvsc_packet *packet)
{
	char *start = net_device->send_buf;
	char *dest = (start + (section_index * net_device->send_section_size));
	int i;
	u32 msg_size = 0;

	for (i = 0; i < packet->page_buf_cnt; i++) {
		char *src = phys_to_virt(packet->page_buf[i].pfn << PAGE_SHIFT);
		u32 offset = packet->page_buf[i].offset;
		u32 len = packet->page_buf[i].len;

		memcpy(dest, (src + offset), len);
		msg_size += len;
		dest += len;
	}
	return msg_size;
}

int netvsc_send(struct hv_device *device,
			struct hv_netvsc_packet *packet)
{
	struct netvsc_device *net_device;
	int ret = 0;
	struct nvsp_message sendMessage;
	struct net_device *ndev;
	struct vmbus_channel *out_channel = NULL;
	u64 req_id;
	unsigned int section_index = NETVSC_INVALID_INDEX;
	u32 msg_size = 0;
	struct sk_buff *skb;


	net_device = get_outbound_net_device(device);
	if (!net_device)
		return -ENODEV;

	/* We may race with netvsc_connect_vsp()/netvsc_init_buf() and get
	 * here before the negotiation with the host is finished and
	 * send_section_map may not be allocated yet.
	 */
	if (!net_device->send_section_map)
		return -EAGAIN;

	ndev = hv_get_drvdata(device);

	sendMessage.hdr.msg_type = NVSP_MSG1_TYPE_SEND_RNDIS_PKT;
	if (packet->is_data_pkt) {
		/* 0 is RMC_DATA; */
		sendMessage.msg.v1_msg.send_rndis_pkt.channel_type = 0;
	} else {
		/* 1 is RMC_CONTROL; */
		sendMessage.msg.v1_msg.send_rndis_pkt.channel_type = 1;
	}

	/* Attempt to send via sendbuf */
	if (packet->total_data_buflen < net_device->send_section_size) {
		section_index = netvsc_get_next_send_section(net_device);
		if (section_index != NETVSC_INVALID_INDEX) {
			msg_size = netvsc_copy_to_send_buf(net_device,
							   section_index,
							   packet);
			skb = (struct sk_buff *)
			      (unsigned long)packet->send_completion_tid;
			if (skb)
				dev_kfree_skb_any(skb);
			packet->page_buf_cnt = 0;
		}
	}
	packet->send_buf_index = section_index;


	sendMessage.msg.v1_msg.send_rndis_pkt.send_buf_section_index =
		section_index;
	sendMessage.msg.v1_msg.send_rndis_pkt.send_buf_section_size = msg_size;

	if (packet->send_completion)
		req_id = (ulong)packet;
	else
		req_id = 0;

	out_channel = net_device->chn_table[packet->q_idx];
	if (out_channel == NULL)
		out_channel = device->channel;
	packet->channel = out_channel;

	if (packet->page_buf_cnt) {
		ret = vmbus_sendpacket_pagebuffer(out_channel,
						  packet->page_buf,
						  packet->page_buf_cnt,
						  &sendMessage,
						  sizeof(struct nvsp_message),
						  req_id);
	} else {
		ret = vmbus_sendpacket(out_channel, &sendMessage,
				sizeof(struct nvsp_message),
				req_id,
				VM_PKT_DATA_INBAND,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	}

	if (ret == 0) {
		atomic_inc(&net_device->num_outstanding_sends);
		atomic_inc(&net_device->queue_sends[packet->q_idx]);

		if (hv_ringbuf_avail_percent(&out_channel->outbound) <
			RING_AVAIL_PERCENT_LOWATER) {
			netif_tx_stop_queue(netdev_get_tx_queue(
					    ndev, packet->q_idx));

			if (atomic_read(&net_device->
				queue_sends[packet->q_idx]) < 1)
				netif_tx_wake_queue(netdev_get_tx_queue(
						    ndev, packet->q_idx));
		}
	} else if (ret == -EAGAIN) {
		netif_tx_stop_queue(netdev_get_tx_queue(
				    ndev, packet->q_idx));
		if (atomic_read(&net_device->queue_sends[packet->q_idx]) < 1) {
			netif_tx_wake_queue(netdev_get_tx_queue(
					    ndev, packet->q_idx));
			ret = -ENOSPC;
		}
	} else {
		netdev_err(ndev, "Unable to send packet %p ret %d\n",
			   packet, ret);
	}

	return ret;
}

static void netvsc_send_recv_completion(struct hv_device *device,
					struct vmbus_channel *channel,
					struct netvsc_device *net_device,
					u64 transaction_id, u32 status)
{
	struct nvsp_message recvcompMessage;
	int retries = 0;
	int ret;
	struct net_device *ndev = hv_get_drvdata(device);

	recvcompMessage.hdr.msg_type =
				NVSP_MSG1_TYPE_SEND_RNDIS_PKT_COMPLETE;

	recvcompMessage.msg.v1_msg.send_rndis_pkt_complete.status = status;

retry_send_cmplt:
	/* Send the completion */
	ret = vmbus_sendpacket(channel, &recvcompMessage,
			       sizeof(struct nvsp_message), transaction_id,
			       VM_PKT_COMP, 0);
	if (ret == 0) {
		/* success */
		/* no-op */
	} else if (ret == -EAGAIN) {
		/* no more room...wait a bit and attempt to retry 3 times */
		retries++;
		netdev_err(ndev, "unable to send receive completion pkt"
			" (tid %llx)...retrying %d\n", transaction_id, retries);

		if (retries < 4) {
			udelay(100);
			goto retry_send_cmplt;
		} else {
			netdev_err(ndev, "unable to send receive "
				"completion pkt (tid %llx)...give up retrying\n",
				transaction_id);
		}
	} else {
		netdev_err(ndev, "unable to send receive "
			"completion pkt - %llx\n", transaction_id);
	}
}

static void netvsc_receive(struct netvsc_device *net_device,
			struct vmbus_channel *channel,
			struct hv_device *device,
			    struct vmpacket_descriptor *packet)
{
	struct vmtransfer_page_packet_header *vmxferpage_packet;
	struct nvsp_message *nvsp_packet;
	struct hv_netvsc_packet nv_pkt;
	struct hv_netvsc_packet *netvsc_packet = &nv_pkt;
	u32 status = NVSP_STAT_SUCCESS;
	int i;
	int count = 0;
	struct net_device *ndev = hv_get_drvdata(device);

	/*
	 * All inbound packets other than send completion should be xfer page
	 * packet
	 */
	if (packet->type != VM_PKT_DATA_USING_XFER_PAGES) {
		netdev_err(ndev, "Unknown packet type received - %d\n",
			   packet->type);
		return;
	}

	nvsp_packet = (struct nvsp_message *)((unsigned long)packet +
			(packet->offset8 << 3));

	/* Make sure this is a valid nvsp packet */
	if (nvsp_packet->hdr.msg_type !=
	    NVSP_MSG1_TYPE_SEND_RNDIS_PKT) {
		netdev_err(ndev, "Unknown nvsp packet type received-"
			" %d\n", nvsp_packet->hdr.msg_type);
		return;
	}

	vmxferpage_packet = (struct vmtransfer_page_packet_header *)packet;

	if (vmxferpage_packet->xfer_pageset_id != NETVSC_RECEIVE_BUFFER_ID) {
		netdev_err(ndev, "Invalid xfer page set id - "
			   "expecting %x got %x\n", NETVSC_RECEIVE_BUFFER_ID,
			   vmxferpage_packet->xfer_pageset_id);
		return;
	}

	count = vmxferpage_packet->range_cnt;
	netvsc_packet->device = device;
	netvsc_packet->channel = channel;

	/* Each range represents 1 RNDIS pkt that contains 1 ethernet frame */
	for (i = 0; i < count; i++) {
		/* Initialize the netvsc packet */
		netvsc_packet->status = NVSP_STAT_SUCCESS;
		netvsc_packet->data = (void *)((unsigned long)net_device->
			recv_buf + vmxferpage_packet->ranges[i].byte_offset);
		netvsc_packet->total_data_buflen =
					vmxferpage_packet->ranges[i].byte_count;

		/* Pass it to the upper layer */
		rndis_filter_receive(device, netvsc_packet);

		if (netvsc_packet->status != NVSP_STAT_SUCCESS)
			status = NVSP_STAT_FAIL;
	}

	netvsc_send_recv_completion(device, channel, net_device,
				    vmxferpage_packet->d.trans_id, status);
}


static void netvsc_send_table(struct hv_device *hdev,
			      struct vmpacket_descriptor *vmpkt)
{
	struct netvsc_device *nvscdev;
	struct net_device *ndev = hv_get_drvdata(hdev);
	struct nvsp_message *nvmsg;
	int i;
	u32 count, *tab;

	nvscdev = get_outbound_net_device(hdev);
	if (!nvscdev)
		return;

	nvmsg = (struct nvsp_message *)((unsigned long)vmpkt +
					(vmpkt->offset8 << 3));

	if (nvmsg->hdr.msg_type != NVSP_MSG5_TYPE_SEND_INDIRECTION_TABLE)
		return;

	count = nvmsg->msg.v5_msg.send_table.count;
	if (count != VRSS_SEND_TAB_SIZE) {
		netdev_err(ndev, "Received wrong send-table size:%u\n", count);
		return;
	}

	tab = (u32 *)((unsigned long)&nvmsg->msg.v5_msg.send_table +
		      nvmsg->msg.v5_msg.send_table.offset);

	for (i = 0; i < count; i++)
		nvscdev->send_table[i] = tab[i];
}

void netvsc_channel_cb(void *context)
{
	int ret;
	struct vmbus_channel *channel = (struct vmbus_channel *)context;
	struct hv_device *device;
	struct netvsc_device *net_device;
	u32 bytes_recvd;
	u64 request_id;
	struct vmpacket_descriptor *desc;
	unsigned char *buffer;
	int bufferlen = NETVSC_PACKET_SIZE;
	struct net_device *ndev;

	if (channel->primary_channel != NULL)
		device = channel->primary_channel->device_obj;
	else
		device = channel->device_obj;

	net_device = get_inbound_net_device(device);
	if (!net_device)
		return;
	ndev = hv_get_drvdata(device);
	buffer = get_per_channel_state(channel);

	/* commit_rd_index() -> hv_signal_on_read() needs this. */
	init_cached_read_index(channel);

	do {
		ret = vmbus_recvpacket_raw(channel, buffer, bufferlen,
					   &bytes_recvd, &request_id);
		if (ret == 0) {
			if (bytes_recvd > 0) {
				desc = (struct vmpacket_descriptor *)buffer;
				switch (desc->type) {
				case VM_PKT_COMP:
					netvsc_send_completion(net_device,
								device, desc);
					break;

				case VM_PKT_DATA_USING_XFER_PAGES:
					netvsc_receive(net_device, channel,
						       device, desc);
					break;

				case VM_PKT_DATA_INBAND:
					netvsc_send_table(device, desc);
					break;

				default:
					netdev_err(ndev,
						   "unhandled packet type %d, "
						   "tid %llx len %d\n",
						   desc->type, request_id,
						   bytes_recvd);
					break;
				}

			} else {
				/*
				 * We are done for this pass.
				 */
				break;
			}

		} else if (ret == -ENOBUFS) {
			if (bufferlen > NETVSC_PACKET_SIZE)
				kfree(buffer);
			/* Handle large packet */
			buffer = kmalloc(bytes_recvd, GFP_ATOMIC);
			if (buffer == NULL) {
				/* Try again next time around */
				netdev_err(ndev,
					   "unable to allocate buffer of size "
					   "(%d)!!\n", bytes_recvd);
				break;
			}

			bufferlen = bytes_recvd;
		}

		init_cached_read_index(channel);

	} while (1);

	if (bufferlen > NETVSC_PACKET_SIZE)
	kfree(buffer);
	return;
}

/*
 * netvsc_device_add - Callback when the device belonging to this
 * driver is added
 */
int netvsc_device_add(struct hv_device *device, void *additional_info)
{
	int i, ret = 0;
	int ring_size =
	((struct netvsc_device_info *)additional_info)->ring_size;
	struct netvsc_device *net_device;
	struct net_device *ndev = hv_get_drvdata(device);
	struct net_device_context *net_device_ctx = netdev_priv(ndev);


	net_device = alloc_net_device();
	if (!net_device) {
		ret = -ENOMEM;
		goto cleanup;
	}

	net_device->ring_size = ring_size;

	/* Initialize the NetVSC channel extension */
	init_completion(&net_device->channel_init_wait);

	set_per_channel_state(device->channel, net_device->cb_buffer);

	/* Open the channel */
	ret = vmbus_open(device->channel, ring_size * PAGE_SIZE,
			 ring_size * PAGE_SIZE, NULL, 0,
			 netvsc_channel_cb, device->channel);

	if (ret != 0) {
		netdev_err(ndev, "unable to open channel: %d\n", ret);
		goto cleanup;
	}

	/* Channel is opened */
	pr_info("hv_netvsc channel opened successfully\n");

	/* If we're reopening the device we may have multiple queues, fill the
	 * chn_table with the default channel to use it before subchannels are
	 * opened.
	 */
	for (i = 0; i < VRSS_CHANNEL_MAX; i++)
		net_device->chn_table[i] = device->channel;

	/* Writing nvdev pointer unlocks netvsc_send(), make sure chn_table is
	 * populated.
	 */
	wmb();

	net_device_ctx->nvdev = net_device;

	/* Connect with the NetVsp */
	ret = netvsc_connect_vsp(device, net_device,
				 (struct netvsc_device_info *)additional_info);
	if (ret != 0) {
		netdev_err(ndev,
			"unable to connect to NetVSP - %d\n", ret);
		goto close;
	}

	return ret;

close:
	/* Now, we can close the channel safely */
	vmbus_close(device->channel);

cleanup:

	if (net_device)
		free_netvsc_device(net_device);

	return ret;
}
