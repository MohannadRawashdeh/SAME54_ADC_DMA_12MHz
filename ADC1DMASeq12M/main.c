#include <atmel_start.h>
#include "hpl_dma.h"

#define DMAC_CHANNEL_ADC_SEQ 0U
#define DMAC_CHANNEL_ADC_SRAM 1U

/*
 * Input pins used are  AIN5, AIN6 and AIN7,AIN8
 */
uint32_t              adc_seq_regs[4] = {0x1805, 0x1806, 0x1807, 0x1808};
volatile uint16_t     adc_res[4]      = {0};
volatile bool         adc_dma_done    = 0;
struct _dma_resource *adc_sram_dma_resource;

void adc_sram_dma_callback(struct _dma_resource *adc_dma_res)
{
	adc_dma_done = true;
}

void adc_dmac_sequence_init()
{
	/* Configure the DMAC source address, destination address,
	 * next descriptor address, data count and Enable the DMAC Channel */
	_dma_set_source_address(DMAC_CHANNEL_ADC_SEQ, (const void *)adc_seq_regs);
	_dma_set_destination_address(DMAC_CHANNEL_ADC_SEQ, (const void *)&ADC1->DSEQDATA.reg);
	_dma_set_data_amount(DMAC_CHANNEL_ADC_SEQ, 4);
	_dma_enable_transaction(DMAC_CHANNEL_ADC_SEQ, false);
}

void adc_sram_dmac_init()
{
	_dma_set_source_address(DMAC_CHANNEL_ADC_SRAM, (const void *)&ADC1->RESULT.reg);
	_dma_set_destination_address(DMAC_CHANNEL_ADC_SRAM, (const void *)adc_res);
	_dma_set_data_amount(DMAC_CHANNEL_ADC_SRAM, 4);
	_dma_set_irq_state(DMAC_CHANNEL_ADC_SRAM, DMA_TRANSFER_COMPLETE_CB, true);
	_dma_get_channel_resource(&adc_sram_dma_resource, DMAC_CHANNEL_ADC_SRAM);
	adc_sram_dma_resource[0].dma_cb.transfer_done = adc_sram_dma_callback;
	_dma_enable_transaction(DMAC_CHANNEL_ADC_SRAM, false);
}

int main(void)
{
	atmel_start_init();
	timer_start(&TIMER_0);
	adc_sram_dmac_init();
	adc_dmac_sequence_init();
	printf("\r\n ADC DMA Sequencing example \r\n");
	adc_sync_enable_channel(&ADC_1, 0);
	hri_adc_set_DSEQCTRL_INPUTCTRL_bit(ADC1);

	while (1) {
		if (adc_dma_done) {
			adc_dma_done = false;
			/* If ADC to SRAM transfer is complete, re enable the channels */
			hri_dmac_set_CHCTRLA_ENABLE_bit(DMAC, DMAC_CHANNEL_ADC_SEQ);
			hri_dmac_set_CHCTRLA_ENABLE_bit(DMAC, DMAC_CHANNEL_ADC_SRAM);
			printf("\r\n ADC conversion of 4 inputs done \r\n");
			printf("AIN5: %04d\r\n", adc_res[0]);
			printf("AIN6: %04d\r\n", adc_res[1]);
			printf("AIN7: %04d\r\n", adc_res[2]);
			printf("AIN8: %04d\r\n", adc_res[3]);
		}
	}
}
