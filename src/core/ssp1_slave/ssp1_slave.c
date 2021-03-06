/**************************************************************************/
/*!
    @file     ssp1_slave.c
    @author   K. Townsend (microBuilder.eu)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2012, K. Townsend (microBuilder.eu)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#include "projectconfig.h"

#include "core/gpio/gpio.h"
#include "core/ssp1_slave/ssp1_slave.h"

static uint8_t* ssp1_recv_buff;
static uint32_t ssp1_recv_remainlen;
static SSP_CALLBACK ssp1_recv_callback;

/**************************************************************************/
/*!

*/
/**************************************************************************/
void SSP1_IRQHandler(void)
{
  uint32_t status_register = LPC_SSP1->MIS;
  if(status_register & SSP1_RX_INTERRUPT_MASK)
  {
    if((ssp1_recv_buff != NULL))
    {
      if(ssp1_recv_remainlen > 0)
      {
        *ssp1_recv_buff = LPC_SSP0->DR;
        ssp1_recv_buff++;
        ssp1_recv_remainlen--;
      }
      else
      {
        /* disable rx interrupt. */
        LPC_SSP1->IMSC &= ~SSP1_RX_INTERRUPT_MASK;
        /* call done callback function. */
        ssp1_recv_callback();
        /* reset internal variable. */
        ssp1_recv_buff = NULL;
        ssp1_recv_remainlen = 0;
      }
    }
    else
    {
      uint32_t Dummy = LPC_SSP1->DR;
      (void)Dummy;
    }
  }

  if(status_register & SSP1_RX_INTERRUPT_CLEAR_MASK)
  {
    LPC_SSP1->ICR = status_register;
  }
}

/**************************************************************************/
/*!
    Set SSP clock to 6.0 MHz
*/
/**************************************************************************/
void ssp1_slaveSetupClock()
{
  /* Divide by 1 for SSPCLKDIV */
  LPC_SYSCON->SSP1CLKDIV = SCB_CLKDIV_DIV1;

  /* (PCLK / (CPSDVSR * [SCR+1])) = (72,000,000 / (2 * [5 + 1])) = 6.0 MHz */
  LPC_SSP1->CR0 = ( (7u << 0)     // Data size = 8-bit  (bits 3:0)
           | (0 << 4)             // Frame format = SPI (bits 5:4)
           #if CFG_SSP_CPOL1 == 1
           | (1  << 6)            // CPOL = 1           (bit 6)
           #else
           | (0  << 6)            // CPOL = 0           (bit 6)
           #endif
           #if CFG_SSP_CPHA1 == 1
           | (1 << 7)             // CPHA = 1           (bit 7)
           #else
           | (0 << 7)             // CPHA = 0           (bit 7)
           #endif
           | SSP1_SCR_5);         // Clock rate = 5     (bits 15:8)

  /* Clock prescale register must be even and at least 2 in master mode */
  LPC_SSP1->CPSR = 2;
}

/**************************************************************************/
/*!
    @brief Initialise SSP1
*/
/**************************************************************************/
void ssp1_slaveInit(void)
{
  uint8_t i, Dummy=Dummy;

  /* Reset SSP */
  LPC_SYSCON->PRESETCTRL |= (0x1<<2);

  /* Enable AHB clock to the SSP domain. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 18);

  #if CFG_SSP_MOSI1_LOCATION == CFG_SSP_MOSI1_0_21
    /* Set P0.21 to SSP MOSI1 */
    LPC_IOCON->PIO0_21 &= ~0x07;
    LPC_IOCON->PIO0_21 |= 0x02;
  #elif CFG_SSP_MOSI1_LOCATION == CFG_SSP_MOSI1_1_22
    /* Set P1.22 to SSP MOSI1 */
    LPC_IOCON->PIO1_22 &= ~0x07;
    LPC_IOCON->PIO1_22 |= 0x02;
  #endif

  #if CFG_SSP_MISO1_LOCATION == CFG_SSP_MISO1_0_22
    /* Set P0.22 to SSP MISO1 */
    LPC_IOCON->PIO0_22 &= ~0x07;
    LPC_IOCON->PIO0_22 |= (0x03); // | (0<<3) | (1<<7);   // MISO1, No pull-up/down, ADMODE = digital
  #elif CFG_SSP_MISO1_LOCATION == CFG_SSP_MISO1_1_21
    /* Set P1.21 to SSP MISO1 */
    LPC_IOCON->PIO1_21 &= ~0x07;
    LPC_IOCON->PIO1_21 |= 0x02;
  #endif

  #if CFG_SSP_SCK1_LOCATION == CFG_SSP_SCK1_1_20
    /* Set 1.20 to SSP SCK1 */
    LPC_IOCON->PIO1_20 &= ~0x07;
    LPC_IOCON->PIO1_20 |= 0x02;
  #elif CFG_SSP_SCK1_LOCATION == CFG_SSP_SCK1_1_15
    /* Set 1.15 to SSP SCK1 */
    LPC_IOCON->PIO1_15 &= ~0x07;
    LPC_IOCON->PIO1_15 |= 0x03;
  #else
    #error "Invalid CFG_SSP_SCK1_LOCATION"
  #endif

  /* Set SPI clock to high-speed by default */
  ssp1_slaveSetupClock();

  /* Clear the Rx FIFO */
  for ( i = 0; i < SSP1_FIFOSIZE; i++ )
  {
    Dummy = LPC_SSP1->DR;
  }

  /* Enable device and set it to slave mode, no loopback */
  LPC_SSP1->CR1 = SSP1_CR1_SSE_ENABLED | SSP1_CR1_MS_SLAVE | SSP1_CR1_LBM_NORMAL;
  ssp1_recv_buff = NULL;
  ssp1_recv_remainlen = 0;
  NVIC_EnableIRQ(SSP1_IRQn);
}

/**************************************************************************/
/*!
    @brief Sends and receive a block of data using SSP1

    @param[in]  recvbufbuf
                Pointer to the rx data buffer
    @param[in]  sendbuf
                Pointer to the tx data buffer
    @param[in]  length
                Block length of the data buffer
*/
/**************************************************************************/
void ssp1_slaveTransfer(uint8_t *recvbuf, uint8_t *sendbuf, uint32_t length)
{
  uint32_t i;
  uint32_t Dummy;

  for (i = 0; i < length; i++)
  {
    /* Move on only if NOT busy and TX FIFO not full. */
    while ((LPC_SSP1->SR & (SSP1_SR_TNF_NOTFULL | SSP1_SR_BSY_BUSY)) != SSP1_SR_TNF_NOTFULL);
    if(sendbuf != NULL)
    {
      LPC_SSP1->DR = *sendbuf;
      sendbuf++;
    }
    else
    {
      LPC_SSP1->DR = 0xFF;
    }

    while ( (LPC_SSP1->SR & (/*SSP1_SR_BSY_BUSY|*/SSP1_SR_RNE_NOTEMPTY)) != SSP1_SR_RNE_NOTEMPTY );
    /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
    on MISO. Otherwise, when this function is called, previous data byte
    is left in the FIFO. */
    if(recvbuf != NULL)
    {
      *recvbuf = LPC_SSP1->DR;
      recvbuf++;
    }
    else
    {
      Dummy = LPC_SSP1->DR;
      (void)Dummy;
    }
  }
}

/**************************************************************************/
/*!
    @brief receive a block of data using SSP0

    @param[in]  buf
                Pointer to the data buffer
    @param[in]  len
                length of the data buffer
    @param[in]  callback
                Function pointer to be called when done.
    @return     Number of received data.
*/
/**************************************************************************/
void ssp1_slaveInterruptRecv(uint8_t* buf, uint32_t len, SSP_CALLBACK callback)
{
  ssp1_recv_buff = buf;
  ssp1_recv_remainlen = len;
  ssp1_recv_callback = callback;
  /* set interrupt on recv. */
  LPC_SSP1->IMSC |= SSP1_RX_INTERRUPT_MASK;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void ssp1_slave_send(uint8_t const * buf, uint32_t length)
{
  ssp1_slaveTransfer(NULL, (uint8_t*)buf, length);
}
