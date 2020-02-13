/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     serial port definition for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#define RT2880_UART_RBR_OFFSET  0x00
#define RT2880_UART_TBR_OFFSET  0x00
#define RT2880_UART_IER_OFFSET  0x04
#define RT2880_UART_IIR_OFFSET  0x08
#define RT2880_UART_FCR_OFFSET  0x08
#define RT2880_UART_LCR_OFFSET  0x0C
#define RT2880_UART_MCR_OFFSET  0x10
#define RT2880_UART_LSR_OFFSET  0x14
#define RT2880_UART_DLL_OFFSET  0x00
#define RT2880_UART_DLM_OFFSET  0x04

#define RBR(x)          *(volatile u32 *)((x)+RT2880_UART_RBR_OFFSET)
#define TBR(x)          *(volatile u32 *)((x)+RT2880_UART_TBR_OFFSET)
#define IER(x)          *(volatile u32 *)((x)+RT2880_UART_IER_OFFSET)
#define IIR(x)          *(volatile u32 *)((x)+RT2880_UART_IIR_OFFSET)
#define FCR(x)          *(volatile u32 *)((x)+RT2880_UART_FCR_OFFSET)
#define LCR(x)          *(volatile u32 *)((x)+RT2880_UART_LCR_OFFSET)
#define MCR(x)          *(volatile u32 *)((x)+RT2880_UART_MCR_OFFSET)
#define LSR(x)          *(volatile u32 *)((x)+RT2880_UART_LSR_OFFSET)
#define DLL(x)          *(volatile u32 *)((x)+RT2880_UART_DLL_OFFSET)
#define DLM(x)          *(volatile u32 *)((x)+RT2880_UART_DLM_OFFSET)


#define UART_RX		0	/* In:  Receive buffer */
#define UART_TX		0	/* Out: Transmit buffer */
#define UART_DLL	0	/* Out: Divisor Latch Low */
#define UART_TRG	0	/* FCTR bit 7 selects Rx or Tx
				 * In: Fifo count
				 * Out: Fifo custom trigger levels */

#define UART_DLM	4	/* Out: Divisor Latch High */
#define UART_IER	4	/* Out: Interrupt Enable Register */
#define UART_FCTR	4	/* Feature Control Register */

#define UART_IIR	8	/* In:  Interrupt ID Register */
#define UART_FCR	8	/* Out: FIFO Control Register */
#define UART_EFR	8	/* I/O: Extended Features Register */

#define UART_LCR	12	/* Out: Line Control Register */
#define UART_MCR	16	/* Out: Modem Control Register */
#define UART_LSR	20	/* In:  Line Status Register */
#define UART_MSR	24	/* In:  Modem Status Register */
#define UART_SCR	28	/* I/O: Scratch Register */
#define UART_EMSR	28	/* Extended Mode Select Register */

/*
 * DLAB=0
 */
//#define UART_IER	1	/* Out: Interrupt Enable Register */
#define UART_IER_MSI		0x08 /* Enable Modem status interrupt */
#define UART_IER_RLSI		0x04 /* Enable receiver line status interrupt */
#define UART_IER_THRI		0x02 /* Enable Transmitter holding register int. */
#define UART_IER_RDI		0x01 /* Enable receiver data interrupt */
/*
 * Sleep mode for ST16650 and TI16750.  For the ST16650, EFR[4]=1
 */
#define UART_IERX_SLEEP		0x10 /* Enable sleep mode */

//#define UART_IIR	2	/* In:  Interrupt ID Register */
#define UART_IIR_NO_INT		0x01 /* No interrupts pending */
#define UART_IIR_ID		0x06 /* Mask for the interrupt ID */
#define UART_IIR_MSI		0x00 /* Modem status interrupt */
#define UART_IIR_THRI		0x02 /* Transmitter holding register empty */
#define UART_IIR_RDI		0x04 /* Receiver data interrupt */
#define UART_IIR_RLSI		0x06 /* Receiver line status interrupt */

//#define UART_LCR	3	/* Out: Line Control Register */
/*
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting 
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_DLAB		0x80 /* Divisor latch access bit */
#define UART_LCR_SBC		0x40 /* Set break control */
#define UART_LCR_SPAR		0x20 /* Stick parity (?) */
#define UART_LCR_EPAR		0x10 /* Even parity select */
#define UART_LCR_PARITY		0x08 /* Parity Enable */
#define UART_LCR_STOP		0x04 /* Stop bits: 0=1 bit, 1=2 bits */
#define UART_LCR_WLEN5		0x00 /* Wordlength: 5 bits */
#define UART_LCR_WLEN6		0x01 /* Wordlength: 6 bits */
#define UART_LCR_WLEN7		0x02 /* Wordlength: 7 bits */
#define UART_LCR_WLEN8		0x03 /* Wordlength: 8 bits */

//#define UART_LSR	5	/* In:  Line Status Register */
#define UART_LSR_TEMT		0x40 /* Transmitter empty */
#define UART_LSR_THRE		0x20 /* Transmit-hold-register empty */
#define UART_LSR_BI		0x10 /* Break interrupt indicator */
#define UART_LSR_FE		0x08 /* Frame error indicator */
#define UART_LSR_PE		0x04 /* Parity error indicator */
#define UART_LSR_OE		0x02 /* Overrun error indicator */
#define UART_LSR_DR		0x01 /* Receiver data ready */
