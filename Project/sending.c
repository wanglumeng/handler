#include "clock.h"
#include "handler.h"
#include "sending.h"

uint8_t cmd_buf[BUF_SIZE] = {0};

float roll_rad = 0, kowtow_rad = 0;

struct key keys[12] = {0};

void sending_config(void)
{
	keys[LU_KEY].data_pos = 4;
	keys[LD_KEY].data_pos = 4;
	keys[LL_KEY].data_pos = 4;
	keys[LR_KEY].data_pos = 4;
	keys[RU_KEY].data_pos = 5;
	keys[RD_KEY].data_pos = 5;
	keys[RL_KEY].data_pos = 5;
	keys[RR_KEY].data_pos = 5;
	keys[L1_KEY].data_pos = 5;
	keys[L2_KEY].data_pos = 5;
	keys[R1_KEY].data_pos = 5;
	keys[R2_KEY].data_pos = 5;

	keys[LU_KEY].id = 0x10;
	keys[LD_KEY].id = 0x40;
	keys[LL_KEY].id = 0x80;
	keys[LR_KEY].id = 0x20;
	keys[RU_KEY].id = 0x10;
	keys[RD_KEY].id = 0x40;
	keys[RL_KEY].id = 0x80;
	keys[RR_KEY].id = 0x20;
	keys[L1_KEY].id = 0x04;
	keys[L2_KEY].id = 0x01;
	keys[R1_KEY].id = 0x08;
	keys[R2_KEY].id = 0x02;
}

void send_cmd(void)
{
	uint8_t i;
	uint8_t cmd_len;

	// we don't use command 0x00
	if(cmd_buf[0] == 0x00)
		return;
	
	cmd_len = ((cmd_buf[0] & 0xf0) >> 4) + 2;

	#ifdef DEBUG
	printf("sending cmd:");
	for(i = 0; i < cmd_len; i++) {
		printf("0x%x\t", cmd_buf[i]);
	}
	printf("\n");
	#endif

	for(i = 0; i < cmd_len; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, (uint8_t) cmd_buf[i]);
		cmd_buf[i] = 0;
	}
}

void send_control_data(void)
{
	uint8_t cmd;
	int8_t spd_x, spd_y, r_spd;
	const int8_t spd = 50;
	uint8_t tmp, tmp1;
	uint8_t check_sum;


	

	{
		r_spd = data[8] - 0x80;
		
		if(ABS(r_spd) > HAND_ZERO) {
			static uint8_t rotate_count;
			rotate_count++;
			if(rotate_count < CMD_TIMES)
				return;

			rotate_count = 0;

			cmd = 0x10;
			cmd_buf[0] = cmd;
			cmd_buf[1] = r_spd;
			cmd_buf[2] = cmd + r_spd;
			send_cmd();
		
			#ifdef DEBUG
			printf("cmd:0x%x\tr_spd:0x%x\n", (uint8_t)cmd, (uint8_t)r_spd);
			#endif
		} else {
			spd_x = data[6] - 0x80;
			spd_y = 0x7f - data[7];

			if(ABS(spd_x) > HAND_ZERO || ABS(spd_y) > HAND_ZERO) {
				static uint8_t move_count = 0;
				move_count++;
				if(move_count < CMD_TIMES)
					return;
				
				move_count = 0;
				
				if(ABS(spd_x) > ABS(spd_y)) {
					
					#ifdef DEBUG
					printf("spd_x = %d > spd_y = %d\n", spd_x, spd_y);
					#endif
					
					cmd = 0x22;
					
					if(spd_x > 0)
						tmp = spd;
					else
						tmp = (uint8_t)(-spd);
					
					tmp1 = 0;
					check_sum = cmd + tmp + tmp1;
					cmd_buf[0] = cmd;
					cmd_buf[1] = tmp;
					cmd_buf[2] = tmp1;
					cmd_buf[3] = check_sum;
					send_cmd();
					
				} else {
					
					#ifdef DEBUG
					printf("spd_x = %d <= spd_y = %d\n", spd_x, spd_y);
					#endif
					
					cmd = 0x22;
					tmp = 0;
					
					if(spd_y > 0)
						tmp1 = spd;
					else
						tmp1 = (uint8_t)(-spd);
					
					check_sum = cmd + tmp + tmp1;
					cmd_buf[0] = cmd;
					cmd_buf[1] = tmp;
					cmd_buf[2] = tmp1;
					cmd_buf[3] = check_sum;
					send_cmd();
				}
				
				send_cmd();
				
			} else {
				#ifdef DEBUG
				printf("r_spd:%x\tspd_x:%x\tspd_y:%x\n", (uint8_t)r_spd, (uint8_t)spd_x, (uint8_t)spd_y);
				#endif
			}
		}
	}
}