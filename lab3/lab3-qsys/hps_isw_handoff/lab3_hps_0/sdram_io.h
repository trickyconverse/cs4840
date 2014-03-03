#include <sdram.h>

#define MGR_SELECT_MASK   0xf8000

#define APB_BASE_SCC_MGR	SDR_PHYGRP_SCCGRP_ADDRESS
#define APB_BASE_PHY_MGR	SDR_PHYGRP_PHYMGRGRP_ADDRESS
#define APB_BASE_RW_MGR		SDR_PHYGRP_RWMGRGRP_ADDRESS
#define APB_BASE_DATA_MGR	SDR_PHYGRP_DATAMGRGRP_ADDRESS
#define APB_BASE_REG_FILE	SDR_PHYGRP_REGFILEGRP_ADDRESS
#define APB_BASE_MMR		SDR_CTRLGRP_ADDRESS

#define __AVL_TO_APB(ADDR) \
	((((ADDR) & MGR_SELECT_MASK) == (BASE_PHY_MGR))  ? (APB_BASE_PHY_MGR)  | (((ADDR) >> (14-6)) & (0x1<<6))  | ((ADDR) & 0x3f) : \
	 (((ADDR) & MGR_SELECT_MASK) == (BASE_RW_MGR))   ? (APB_BASE_RW_MGR)   | ((ADDR) & 0x1fff) : \
 	 (((ADDR) & MGR_SELECT_MASK) == (BASE_DATA_MGR)) ? (APB_BASE_DATA_MGR) | ((ADDR) & 0x7ff) : \
	 (((ADDR) & MGR_SELECT_MASK) == (BASE_SCC_MGR))  ? (APB_BASE_SCC_MGR)  | ((ADDR) & 0xfff) : \
	 (((ADDR) & MGR_SELECT_MASK) == (BASE_REG_FILE)) ? (APB_BASE_REG_FILE) | ((ADDR) & 0x7ff) : \
	 (((ADDR) & MGR_SELECT_MASK) == (BASE_MMR))      ? (APB_BASE_MMR)      | ((ADDR) & 0xfff) : \
	 -1)

#define IOWR_32DIRECT(BASE, OFFSET, DATA) \
	write_register(HPS_SDR_BASE, __AVL_TO_APB((alt_u32)((BASE) + (OFFSET))), DATA)

#define IORD_32DIRECT(BASE, OFFSET) \
	read_register(HPS_SDR_BASE, __AVL_TO_APB((alt_u32)((BASE) + (OFFSET))))

