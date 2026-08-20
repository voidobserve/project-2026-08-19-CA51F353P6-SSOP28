#ifndef __AIP3368H_MODULE_H__
#define __AIP3368H_MODULE_H__



#define AIP3368H_DEV_NUM   17
#define AIP3368H_DEV_NUM_L 7
#define AIP3368H_DEV_NUM_R 8

extern xdata u16 display_buff[AIP3368H_DEV_NUM_L];
extern xdata u16 display_buff_r[AIP3368H_DEV_NUM_R];

void aip3368h_module_init(void);
void aip3368h_module_uninit(void);
void aip3368h_module_display(void);
void aip3368h_module_clear(void);

#endif
