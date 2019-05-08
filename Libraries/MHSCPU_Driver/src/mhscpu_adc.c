/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : mhscpu_adc.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 09/12/2016
 * Description          : ADC module functions set.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
#include "mhscpu_adc.h"

#define FILT_NUM    20      //�˲�����
#define REMOVE_NUM  6       //��λֵ�˲�ʱͷβ����Ҫȥ��������




volatile uint32_t g_u32ADC_CR = 0;

/******************************************************************************
* Function Name  : ADC_Init
* Description    : ��ʼ��ADC,��ʼ���ο�ֵ
* Input          : ADC_InitStruct��Ҫ��ʼ�������ݽṹָ��
* Return         : NONE
******************************************************************************/
void ADC_Init(ADC_InitTypeDef *ADC_InitStruct)
{
    assert_param(IS_ADC_CHANNEL(ADC_InitStruct->ADC_Channel));
    assert_param(IS_ADC_SAMP(ADC_InitStruct->ADC_SampSpeed));
    assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_IRQ_EN));
	
    /* Select ADC Channel */
    g_u32ADC_CR = ADC_InitStruct->ADC_Channel;
    /* Select ADC Channel Samping */
    g_u32ADC_CR |= ADC_InitStruct->ADC_SampSpeed << 3;
    /* Set ADC Interrupt */
    if (ENABLE == ADC_InitStruct->ADC_IRQ_EN )
    {
        NVIC_EnableIRQ(ADC0_IRQn);
        g_u32ADC_CR |= ADC_IRQ_EN_BIT;
    }
    g_u32ADC_CR |= ADC_START_BIT;
    ADC0->ADC_CR = g_u32ADC_CR;                 //дADC_CR���뽫ADC_START_BITλ��1  
}
/******************************************************************************
* Function Name  : Get_ADC_Result
* Description    : ������ȡADC��Ӧͨ����ֵ,�г�ʱ���
* Input          : ADCͨ����ADC_CHANNEL_0~5
* Return         : -1:��ȡ��ʱ  Other:ADCֵ
******************************************************************************/
int32_t ADC_GetResult(ADC_ChxTypeDef ADC_Channel)
{
    uint32_t u32TimeDelay = 0xffff;
    
    ADC0->ADC_CR = g_u32ADC_CR | ADC_Channel;
    while ((ADC0->ADC_CR & ADC_START_BIT) && --u32TimeDelay);
    if (0UL == u32TimeDelay)
    {
        return -1;
    }
    else
    {
        return ADC0->ADC_SR & 0x3FF;
    }
}
/******************************************************************************
* Function Name  : Get_ADC_Result_Wait
* Description    : ������ȡADC��Ӧͨ����ֵ,ֱ��ת�������ſ��Է���
* Input          : ADCͨ����ADC_CHANNEL_0~5
* Output         : NONE
* Return         : NONE
******************************************************************************/
uint32_t ADC_GetResult_Wait(ADC_ChxTypeDef ADC_Channel)
{
    ADC0->ADC_CR = g_u32ADC_CR | ADC_Channel;
    while((ADC0->ADC_CR & ADC_START_BIT));
    return ADC0->ADC_SR & 0x3FF;
}
/******************************************************************************
* Function Name  : Get_ADC_Voltage
* Description    : ��ȡADC��Ӧͨ���ĵ�ѹֵ
* Input          : ADCͨ����ADC_CHANNEL_0~5
* Return         : ��Ӧͨ���ĵ�ѹֵ(�Ŵ�1000��)
******************************************************************************/
int32_t ADC_GetVoltage(ADC_ChxTypeDef ADC_Channel)
{
    uint32_t u32ADC_Result;
    u32ADC_Result = ADC_GetResult_Wait(ADC_Channel);
    return (u32ADC_Result * ADC_REF_mV / 1024);
}
/******************************************************************************
* Function Name  : Calculate_Voltage
* Description    : ����ADCֵ��Ӧ�ĵ�ѹֵ
* Input          : ADCֵ
* Return         : ��ѹֵ(�Ŵ�1000��)
******************************************************************************/
uint32_t ADC_CalVoltage(uint32_t u32ADC_Value)
{
    return (u32ADC_Value * ADC_REF_mV / 1024);
}
/******************************************************************************
* Function Name  : ADC_Filt
* Description    : ��λֵ+����ƽ���˲�
* Input          : ADCͨ����ADC_CHANNEL_0~5
* Return         : �˲����ADCֵ
******************************************************************************/
uint32_t ADC_Filt(ADC_ChxTypeDef ADC_Channel)
{
    int8_t i,j;
    uint16_t ADC_Buffer[FILT_NUM],tmp;
    
    if (REMOVE_NUM * 2 >= FILT_NUM)
	{
		return 0;
	}
    
    for (i = 0; i < FILT_NUM; i++)
    {
        ADC_Buffer[i] = ADC_GetResult_Wait(ADC_Channel);
    }
    for (j = 1; j < FILT_NUM; j++) //ֱ�Ӳ�������
    {
        tmp = ADC_Buffer[j];
        i = j - 1;
        //ע:��������˳�򲻿ɵ���
        while(i >= 0 && ADC_Buffer[i] > tmp)    
        {
            ADC_Buffer[i + 1] = ADC_Buffer[i];
            i--;
        }
        ADC_Buffer[i + 1] = tmp;
    }

    for (tmp = 0, i = REMOVE_NUM; i < FILT_NUM - REMOVE_NUM; i++)
    {
        tmp += ADC_Buffer[i];
    }
	
    return tmp / (FILT_NUM - 2 * REMOVE_NUM);
}

/******************************************************************************
* Function Name  : ADC_Filt_Average
* Description    : ����ƽ���˲�,
* Input          : ADCͨ����ADC_CHANNEL_0~5
* Return         : �˲����ADCֵ
******************************************************************************/
uint32_t ADC_Filt_Average(ADC_ChxTypeDef ADC_Channel)
{
    int16_t i;
    uint32_t tmp = 0;
    uint16_t ADC_Buffer[FILT_NUM];
	
    for (i = 0; i < FILT_NUM; i++)
    {
        ADC_Buffer[i] = ADC_GetResult_Wait(ADC_Channel);
        tmp += ADC_Buffer[i];
    }
	
    return tmp / (FILT_NUM * 100);
}

/************************ (C) COPYRIGHT 2016 Megahuntmicro ****END OF FILE****/


