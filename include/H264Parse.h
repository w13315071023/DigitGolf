#ifndef H264STREAMPARSE_HH
#define H264STREAMPARSE_HH

#include <math.h>	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Ue(unsigned char *buff, unsigned int len, unsigned int* startbit)
{
	int i;
    unsigned int zeronum = 0;
	unsigned int ret = 0;

	//计算0bit的个数
    while ((*startbit) < (len*8))
    {
        if(buff[(*startbit)/8] & (0x80 >> ((*startbit)%8))){
            break;
        }
        zeronum++;
        (*startbit)++;
    }
    (*startbit)++;

    //计算结果
    for (i=0; i<(int)zeronum; i++)
    {
        ret <<= 1;
        if (buff[(*startbit)/8] & (0x80 >> ((*startbit)%8))){
            ret += 1;
        }
        (*startbit)++;
    }

    return (1 << zeronum) - 1 + ret;
}

int Se(unsigned char* pBuff, unsigned int len, unsigned int* startbit)
{
	int value = 0;
	int UeVal = Ue(pBuff,len,startbit);
	double k = UeVal;
	
	//ceil函数的作用是求不小于给定实数的最小整数,ceil(2)=ceil(1.2)=cei(1.5)=2.00
	value = (int)ceil(k/2);
	if(UeVal % 2 == 0){
		value =- value;
	}

	return value;
}

int u(int bitcount,unsigned char* buf,unsigned int* startbit)
{
	int i;
    unsigned int ret = 0;

	if(buf == NULL || startbit == NULL){
		return -1;
	}

    for (i=0; i<bitcount; i++)
    {
        ret <<= 1;
        if (buf[(*startbit)/8] & (0x80 >> ((*startbit)%8))){
            ret += 1;
        }
        (*startbit)++;
    }//end for

	return ret;
}

int h264_decode_seq_parameter_set(unsigned char* buf,unsigned int len,int* width,int* height)
{
	int i;
	unsigned int startbit=0;
	int frame_crop_left_offset = 0;
	int frame_crop_right_offset = 0;
	int frame_crop_top_offset = 0;
	int frame_crop_bottom_offset = 0;
	int mb_adaptive_frame_field_flag = 0;
	int forbidden_zero_bit = 0;
	int nal_ref_idc=0;
	int nal_unit_type = 0;
	int profile_idc=0;
	int constraint_set0_flag=0;
	int constraint_set1_flag=0;
	int constraint_set2_flag=0;
	int constraint_set3_flag=0;
	int reserved_zero_4bits=0;
	int level_idc=0;
	int seq_parameter_set_id = 0;
	int chroma_format_idc=0;
	int residual_colour_transform_flag=0;
	int bit_depth_luma_minus8=0;
	int bit_depth_chroma_minus8=0;
	int qpprime_y_zero_transform_bypass_flag=0;
	int seq_scaling_matrix_present_flag=0;
	int seq_scaling_list_present_flag[8];
	int log2_max_frame_num_minus4 = 0;
	int pic_order_cnt_type = 0;
	int log2_max_pic_order_cnt_lsb_minus4=0;
	int delta_pic_order_always_zero_flag=0;
	int offset_for_non_ref_pic=0;
	int offset_for_top_to_bottom_field=0;
	int num_ref_frames_in_pic_order_cnt_cycle=0;
	int *offset_for_ref_frame = NULL;
	int num_ref_frames = 0;
	int gaps_in_frame_num_value_allowed_flag = 0;
	int pic_width_in_mbs_minus_1 = 0;
	int pic_height_in_map_units_minus_1 = 0;
	int frame_mbs_only_flag = 0;
	int direct_8x8_inference_flag = 0;
	int frame_cropping_flag = 0;
	int vui_prameters_present_flag = 0;

	if(buf == NULL || width == NULL || height == NULL)
	{
		return -1;
	}
	
	forbidden_zero_bit=u(1,buf,&startbit);
	nal_ref_idc=u(2,buf,&startbit);
	nal_unit_type=u(5,buf,&startbit);
	if(nal_unit_type == 7)
	{
		profile_idc=u(8,buf,&startbit);
		constraint_set0_flag=u(1,buf,&startbit);//(buf[1] & 0x80)>>7;
		constraint_set1_flag=u(1,buf,&startbit);//(buf[1] & 0x40)>>6;
		constraint_set2_flag=u(1,buf,&startbit);//(buf[1] & 0x20)>>5;
		constraint_set3_flag=u(1,buf,&startbit);//(buf[1] & 0x10)>>4;
		reserved_zero_4bits=u(4,buf,&startbit);
		level_idc=u(8,buf,&startbit);
		seq_parameter_set_id = Ue(buf,len,&startbit);
		if(profile_idc == 100 || profile_idc == 110 ||profile_idc == 122 || profile_idc == 144)
		{
			chroma_format_idc=Ue(buf,len,&startbit);
			if(chroma_format_idc == 3){
				residual_colour_transform_flag=u(1,buf,&startbit);
			}
			bit_depth_luma_minus8=Ue(buf,len,&startbit);
			bit_depth_chroma_minus8=Ue(buf,len,&startbit);
			qpprime_y_zero_transform_bypass_flag=u(1,buf,&startbit);
			seq_scaling_matrix_present_flag=u(1,buf,&startbit);
			
			if(seq_scaling_matrix_present_flag)
			{
				for(i=0; i<8; i++) {
					seq_scaling_list_present_flag[i]=u(1,buf,&startbit);
				}
			}
		}

		log2_max_frame_num_minus4 = Ue(buf,len,&startbit);
		pic_order_cnt_type = Ue(buf,len,&startbit);
		if( pic_order_cnt_type == 0 )
		{
			log2_max_pic_order_cnt_lsb_minus4=Ue(buf,len,&startbit);
		}
		else if( pic_order_cnt_type == 1 )
		{
			delta_pic_order_always_zero_flag=u(1,buf,&startbit);
			offset_for_non_ref_pic=Se(buf,len,&startbit);
			offset_for_top_to_bottom_field=Se(buf,len,&startbit);
			num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,len,&startbit);

			offset_for_ref_frame = (int*)malloc(num_ref_frames_in_pic_order_cnt_cycle);
			if(offset_for_ref_frame == NULL){
				return -1;
			}
			for(i=0; i<num_ref_frames_in_pic_order_cnt_cycle; i++){
				offset_for_ref_frame[i]=Se(buf,len,&startbit);
			}
			free(offset_for_ref_frame);
			offset_for_ref_frame = NULL;
		}

		num_ref_frames = Ue(buf,len,&startbit);
		gaps_in_frame_num_value_allowed_flag = u(1,buf,&startbit);
		pic_width_in_mbs_minus_1 = Ue(buf,len,&startbit);
		pic_height_in_map_units_minus_1 = Ue(buf,len,&startbit);
		
		frame_mbs_only_flag = u(1,buf,&startbit);
		if(frame_mbs_only_flag == 0){
			mb_adaptive_frame_field_flag = u(1,buf,&startbit);
		}
		direct_8x8_inference_flag = u(1,buf,&startbit);
		frame_cropping_flag = u(1,buf,&startbit);
		if (frame_cropping_flag)
		{
			frame_crop_left_offset = Ue(buf,len,&startbit);
			frame_crop_right_offset = Ue(buf,len,&startbit);
			frame_crop_top_offset = Ue(buf,len,&startbit);
			frame_crop_bottom_offset = Ue(buf,len,&startbit);
		}
		*width = (pic_width_in_mbs_minus_1+1)*16 - (frame_crop_right_offset * 2 + frame_crop_left_offset * 2);
		*height = (2-frame_mbs_only_flag)*(pic_height_in_map_units_minus_1+1)*16 - (frame_crop_bottom_offset * 2+frame_crop_top_offset * 2);
		return 0;
	}

	return -1;
}
#endif /* __cplusplus */
