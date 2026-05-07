#ifndef _RTK8277BC_REGISTERS_H_
#define _RTK8277BC_REGISTERS_H_

#include "ca_types.h"

/**************** VENUS *************************************/
#define B_GLOBAL_STRAP                             0xf4320038

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t debug_mode           :  1 ; /* bits 0:0 */
    ca_uint32_t speed                :  3 ; /* bits 3:1 */
    ca_uint32_t flash_type           :  3 ; /* bits 6:4 */
    ca_uint32_t flash_size           :  2 ; /* bits 8:7 */
    ca_uint32_t flash_width          :  1 ; /* bits 9:9 */
    ca_uint32_t flash_pin            :  1 ; /* bits 10:10 */
    ca_uint32_t uart_message         :  1 ; /* bits 11:11 */
    ca_uint32_t sd                   :  1 ; /* bits 12:12 */
    ca_uint32_t NO_PON               :  1 ; /* bits 13:13 */
    ca_uint32_t NO_10PON             :  1 ; /* bits 14:14 */
    ca_uint32_t NO_CRYPTO            :  1 ; /* bits 15:15 */
    ca_uint32_t NO_QUAD              :  1 ; /* bits 16:16 */
    ca_uint32_t rsrvd1               : 13 ;
    ca_uint32_t jtag_osc_2           :  1 ; /* bits 30:30 */
    ca_uint32_t jtag_osc_3           :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_STRAP_t;
#endif /* !__ASSEMBLER__ */

#define B_GLOBAL_CPLLDIV                           0xf4320188

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t cortex_divsel        :  6 ; /* bits 5:0 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t cpll_div_override    :  1 ; /* bits 7:7 */
    ca_uint32_t f2c_divsel           :  4 ; /* bits 11:8 */
    ca_uint32_t rsrvd2               :  3 ;
    ca_uint32_t cf_sel               :  1 ; /* bits 15:15 */
    ca_uint32_t trc_divsel           :  7 ; /* bits 22:16 */
    ca_uint32_t rsrvd3               :  8 ;
    ca_uint32_t cpll_mode_override   :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_CPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_CPLLMUX                           0xf432018c

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t cf_i0delay           :  4 ; /* bits 19:16 */
    ca_uint32_t cf_i1delay           :  4 ; /* bits 23:20 */
    ca_uint32_t cf_i0guard           :  4 ; /* bits 27:24 */
    ca_uint32_t cf_i1guard           :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_CPLLMUX_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_EPLLDIV                           0xf4320190

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t crypto_divsel        :  7 ; /* bits 6:0 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t core_divsel          :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               :  1 ;
    ca_uint32_t core_dynamic_en      :  1 ; /* bits 16:16 */
    ca_uint32_t rsrvd3               :  7 ;
    ca_uint32_t lsaxi_divsel         :  7 ; /* bits 30:24 */
    ca_uint32_t rsrvd4               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_EPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_EPLLDIV2                          0xf4320194

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t cci_divsel           :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_EPLLDIV2_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_PEDIV                             0xf4320198

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t f_divsel             :  6 ; /* bits 5:0 */
    ca_uint32_t offload_sel          :  1 ; /* bits 6:6 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t c2f_divsel           :  6 ; /* bits 13:8 */
    ca_uint32_t rsrvd2               :  2 ;
    ca_uint32_t peaxi_divsel         :  6 ; /* bits 21:16 */
    ca_uint32_t peaxi_fullspeedsel   :  1 ; /* bits 22:22 */
    ca_uint32_t rsrvd3               :  9 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_PEDIV_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_PEMUX                             0xf432019c


#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t offload_i0delay      :  4 ; /* bits 19:16 */
    ca_uint32_t offload_i1delay      :  4 ; /* bits 23:20 */
    ca_uint32_t offload_i0guard      :  4 ; /* bits 27:24 */
    ca_uint32_t offload_i1guard      :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_PEMUX_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_CPLL0                             0xf43201b8

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  1 ; /* bits 8:8 */
    ca_uint32_t SCPU_RSTB            :  1 ; /* bits 9:9 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 12:12 */
    ca_uint32_t rsrvd1               : 19 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_CPLL0_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_CPLL1                             0xf43201bc

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  1 ; /* bits 10:10 */
    ca_uint32_t WD_SET               :  1 ; /* bits 11:11 */
    ca_uint32_t OEB                  :  1 ; /* bits 12:12 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 13:13 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 15:14 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 17:16 */
    ca_uint32_t rsrvd1               : 14 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_CPLL1_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_EPLL0                             0xf43201c0

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  1 ; /* bits 8:8 */
    ca_uint32_t SCPU_RSTB            :  1 ; /* bits 9:9 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 12:12 */
    ca_uint32_t rsrvd1               : 19 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_EPLL0_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_EPLL1                             0xf43201c4

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  1 ; /* bits 10:10 */
    ca_uint32_t WD_SET               :  1 ; /* bits 11:11 */
    ca_uint32_t OEB                  :  1 ; /* bits 12:12 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 13:13 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 15:14 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 17:16 */
    ca_uint32_t rsrvd1               : 14 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_EPLL1_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_FPLL0                             0xf43201c8

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  1 ; /* bits 8:8 */
    ca_uint32_t SCPU_RSTB            :  1 ; /* bits 9:9 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 12:12 */
    ca_uint32_t rsrvd1               : 19 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_FPLL0_t;
#endif /* !__ASSEMBLER__ */


#define B_GLOBAL_FPLL1                             0xf43201cc

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  1 ; /* bits 10:10 */
    ca_uint32_t WD_SET               :  1 ; /* bits 11:11 */
    ca_uint32_t OEB                  :  1 ; /* bits 12:12 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 13:13 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 15:14 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 17:16 */
    ca_uint32_t rsrvd1               : 14 ;
  } bf ;
  ca_uint32_t     wrd ;
} B_GLOBAL_FPLL1_t;
#endif /* !__ASSEMBLER__ */


/**************** TAURUS *************************************/
#define C_GLOBAL_STRAP                             0xf43200ac

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t debug_mode           :  1 ; /* bits 0:0 */
    ca_uint32_t speed                :  3 ; /* bits 3:1 */
    ca_uint32_t flash_type           :  3 ; /* bits 6:4 */
    ca_uint32_t flash_size           :  2 ; /* bits 8:7 */
    ca_uint32_t flash_width          :  1 ; /* bits 9:9 */
    ca_uint32_t flash_pin            :  1 ; /* bits 10:10 */
    ca_uint32_t uart_message         :  1 ; /* bits 11:11 */
    ca_uint32_t sd                   :  1 ; /* bits 12:12 */
    ca_uint32_t pmd                  :  1 ; /* bits 13:13 */
    ca_uint32_t rsrvd1               : 10 ;
    ca_uint32_t jtag_osc             :  8 ; /* bits 31:24 */
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_STRAP_t;
#endif /* !__ASSEMBLER__ */



#define C_GLOBAL_CPLLDIV                           0xf4320200

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t cortex_divsel        :  6 ; /* bits 5:0 */
    ca_uint32_t cpll_div_override    :  1 ; /* bits 6:6 */
    ca_uint32_t f2c_divsel           :  4 ; /* bits 10:7 */
    ca_uint32_t cf_sel               :  1 ; /* bits 11:11 */
    ca_uint32_t trc_divsel           :  7 ; /* bits 18:12 */
    ca_uint32_t cfg_pilcs_per_divsel :  6 ; /* bits 24:19 */
    ca_uint32_t cfg_ext_vsfc_divsel  :  6 ; /* bits 30:25 */
    ca_uint32_t cpll_mode_override   :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_CPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_CPLLMUX                           0xf4320204

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t cf_i0delay           :  4 ; /* bits 19:16 */
    ca_uint32_t cf_i1delay           :  4 ; /* bits 23:20 */
    ca_uint32_t cf_i0guard           :  4 ; /* bits 27:24 */
    ca_uint32_t cf_i1guard           :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_CPLLMUX_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_EPLLDIV                           0xf4320208

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t dw_per_divsel        :  7 ; /* bits 6:0 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t core_divsel          :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               :  1 ;
    ca_uint32_t core_dynamic_en      :  1 ; /* bits 16:16 */
    ca_uint32_t rsrvd3               :  7 ;
    ca_uint32_t lsaxi_divsel         :  7 ; /* bits 30:24 */
    ca_uint32_t rsrvd4               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_EPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_EPLLDIV2                          0xf432020c

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t cci_divsel           :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_EPLLDIV2_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_PEDIV                             0xf4320210

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t f_divsel             :  6 ; /* bits 5:0 */
    ca_uint32_t offload_sel          :  1 ; /* bits 6:6 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t c2f_divsel           :  6 ; /* bits 13:8 */
    ca_uint32_t rsrvd2               :  2 ;
    ca_uint32_t peaxi_divsel         :  6 ; /* bits 21:16 */
    ca_uint32_t peaxi_fullspeedsel   :  1 ; /* bits 22:22 */
    ca_uint32_t rsrvd3               :  9 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_PEDIV_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_PEMUX                             0xf4320214

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t offload_i0delay      :  4 ; /* bits 19:16 */
    ca_uint32_t offload_i1delay      :  4 ; /* bits 23:20 */
    ca_uint32_t offload_i0guard      :  4 ; /* bits 27:24 */
    ca_uint32_t offload_i1guard      :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_PEMUX_t;
#endif /* !__ASSEMBLER__ */




#define C_GLOBAL_CPLL0                             0xf4320230

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t rsrvd1               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_CPLL0_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_CPLL1                             0xf4320234

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t rsrvd1               : 11 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_CPLL1_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_EPLL0                             0xf4320238

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t rsrvd1               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_EPLL0_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_EPLL1                             0xf432023c

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t rsrvd1               : 11 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_EPLL1_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_FPLL0                             0xf4320240

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t SCPU_DIV_DIVN        :  8 ; /* bits 7:0 */
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t rsrvd1               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_FPLL0_t;
#endif /* !__ASSEMBLER__ */


#define C_GLOBAL_FPLL1                             0xf4320244

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t rsrvd1               : 11 ;
  } bf ;
  ca_uint32_t     wrd ;
} C_GLOBAL_FPLL1_t;
#endif /* !__ASSEMBLER__ */


/**************** ELNATH *************************************/

#define F_GLOBAL_STRAP                             0xf43200b4

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t debug_mode           :  1 ; /* bits 0:0 */
    ca_uint32_t speed                :  2 ; /* bits 2:1 */
    ca_uint32_t xtal_sel             :  1 ; /* bits 3:3 */
    ca_uint32_t dram_type            :  1 ; /* bits 4:4 */
    ca_uint32_t flash_type           :  2 ; /* bits 6:5 */
    ca_uint32_t flash_size           :  2 ; /* bits 8:7 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t flash_pin            :  1 ; /* bits 10:10 */
    ca_uint32_t uart_message         :  1 ; /* bits 11:11 */
    ca_uint32_t rsrvd2               :  1 ;
    ca_uint32_t pmd                  :  1 ; /* bits 13:13 */
    ca_uint32_t rsrvd3               : 10 ;
    ca_uint32_t jtag_osc             :  8 ; /* bits 31:24 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_STRAP_t;
#endif


#define F_GLOBAL_CPLLDIV                           0xf4320230

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t cortex_divsel        :  6 ; /* bits 5:0 */
    ca_uint32_t cpll_div_override    :  1 ; /* bits 6:6 */
    ca_uint32_t f2c_divsel           :  4 ; /* bits 10:7 */
    ca_uint32_t cf_sel               :  1 ; /* bits 11:11 */
    ca_uint32_t trc_divsel           :  7 ; /* bits 18:12 */
    ca_uint32_t cfg_pilcs_per_divsel :  6 ; /* bits 24:19 */
    ca_uint32_t cfg_ext_vsfc_divsel  :  6 ; /* bits 30:25 */
    ca_uint32_t cpll_mode_override   :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_CPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_CPLLMUX                           0xf4320234

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t cf_i0delay           :  4 ; /* bits 19:16 */
    ca_uint32_t cf_i1delay           :  4 ; /* bits 23:20 */
    ca_uint32_t cf_i0guard           :  4 ; /* bits 27:24 */
    ca_uint32_t cf_i1guard           :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_CPLLMUX_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_EPLLDIV                           0xf4320238

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t dw_per_divsel        :  7 ; /* bits 6:0 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t core_divsel          :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               :  1 ;
    ca_uint32_t core_dynamic_en      :  1 ; /* bits 16:16 */
    ca_uint32_t rsrvd3               :  7 ;
    ca_uint32_t lsaxi_divsel         :  7 ; /* bits 30:24 */
    ca_uint32_t rsrvd4               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_EPLLDIV_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_EPLLDIV2                          0xf432023c

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t cci_divsel           :  7 ; /* bits 14:8 */
    ca_uint32_t rsrvd2               : 17 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_EPLLDIV2_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_PEDIV                             0xf4320240

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t f_divsel             :  6 ; /* bits 5:0 */
    ca_uint32_t offload_sel          :  1 ; /* bits 6:6 */
    ca_uint32_t rsrvd1               :  1 ;
    ca_uint32_t c2f_divsel           :  6 ; /* bits 13:8 */
    ca_uint32_t rsrvd2               :  2 ;
    ca_uint32_t peaxi_divsel         :  6 ; /* bits 21:16 */
    ca_uint32_t peaxi_fullspeedsel   :  1 ; /* bits 22:22 */
    ca_uint32_t rsrvd3               :  9 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_PEDIV_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_PEMUX                             0xf4320244

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               : 16 ;
    ca_uint32_t offload_i0delay      :  4 ; /* bits 19:16 */
    ca_uint32_t offload_i1delay      :  4 ; /* bits 23:20 */
    ca_uint32_t offload_i0guard      :  4 ; /* bits 27:24 */
    ca_uint32_t offload_i1guard      :  4 ; /* bits 31:28 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_PEMUX_t;
#endif /* !__ASSEMBLER_ */

#define F_GLOBAL_CPLL0                             0xf4320260

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t SCPU_DIV_DIVN        :  9 ; /* bits 23:15 */
    ca_uint32_t rsrvd2               :  8 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_CPLL0_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_CPLL1                             0xf4320264

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t FREF_EDGE            :  1 ; /* bits 21:21 */
    ca_uint32_t VC_THL               :  2 ; /* bits 23:22 */
    ca_uint32_t LPF_CP_PLLA          :  2 ; /* bits 25:24 */
    ca_uint32_t BUF_S_PLLA           :  2 ; /* bits 27:26 */
    ca_uint32_t BUF_GPO_PLLA         :  1 ; /* bits 28:28 */
    ca_uint32_t BUF_GPE_PLLA         :  1 ; /* bits 29:29 */
    ca_uint32_t BUF_CKI_SEL_PLLA     :  1 ; /* bits 30:30 */
    ca_uint32_t rsrvd1               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_CPLL1_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_EPLL0                             0xf4320268

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t SCPU_DIV_DIVN        :  9 ; /* bits 23:15 */
    ca_uint32_t rsrvd2               :  7 ;
    ca_uint32_t epll_mode_override   :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_EPLL0_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_EPLL1                             0xf432026c

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t FREF_EDGE            :  1 ; /* bits 21:21 */
    ca_uint32_t VC_THL               :  2 ; /* bits 23:22 */
    ca_uint32_t LPF_CP_PLLA          :  2 ; /* bits 25:24 */
    ca_uint32_t BUF_S_PLLA           :  2 ; /* bits 27:26 */
    ca_uint32_t BUF_GPO_PLLA         :  1 ; /* bits 28:28 */
    ca_uint32_t BUF_GPE_PLLA         :  1 ; /* bits 29:29 */
    ca_uint32_t BUF_CKI_SEL_PLLA     :  1 ; /* bits 30:30 */
    ca_uint32_t rsrvd1               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_EPLL1_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_FPLL0                             0xf4320270

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t rsrvd1               :  8 ;
    ca_uint32_t SCPU_POW             :  2 ; /* bits 9:8 */
    ca_uint32_t SCPU_RSTB            :  2 ; /* bits 11:10 */
    ca_uint32_t DIV_PREDIV_SEL       :  2 ; /* bits 13:12 */
    ca_uint32_t DIV_PREDIV_BPS       :  1 ; /* bits 14:14 */
    ca_uint32_t SCPU_DIV_DIVN        :  9 ; /* bits 23:15 */
    ca_uint32_t rsrvd2               :  7 ;
    ca_uint32_t fpll_mode_override   :  1 ; /* bits 31:31 */
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_FPLL0_t;
#endif /* !__ASSEMBLER__ */


#define F_GLOBAL_FPLL1                             0xf4320274

#ifndef __ASSEMBLER__
typedef volatile union {
  struct {
    ca_uint32_t ICP                  :  3 ; /* bits 2:0 */
    ca_uint32_t LPF_CS               :  2 ; /* bits 4:3 */
    ca_uint32_t LPF_RS               :  3 ; /* bits 7:5 */
    ca_uint32_t SCPU_DBUG_EN         :  1 ; /* bits 8:8 */
    ca_uint32_t SEL_CCO              :  1 ; /* bits 9:9 */
    ca_uint32_t WD_RST               :  2 ; /* bits 11:10 */
    ca_uint32_t WD_SET               :  2 ; /* bits 13:12 */
    ca_uint32_t OEB                  :  2 ; /* bits 15:14 */
    ca_uint32_t TESTSEL              :  1 ; /* bits 16:16 */
    ca_uint32_t LDO10V_SEL           :  2 ; /* bits 18:17 */
    ca_uint32_t LDO18V_SEL           :  2 ; /* bits 20:19 */
    ca_uint32_t FREF_EDGE            :  1 ; /* bits 21:21 */
    ca_uint32_t VC_THL               :  2 ; /* bits 23:22 */
    ca_uint32_t LPF_CP_PLLA          :  2 ; /* bits 25:24 */
    ca_uint32_t BUF_S_PLLA           :  2 ; /* bits 27:26 */
    ca_uint32_t BUF_GPO_PLLA         :  1 ; /* bits 28:28 */
    ca_uint32_t BUF_GPE_PLLA         :  1 ; /* bits 29:29 */
    ca_uint32_t BUF_CKI_SEL_PLLA     :  1 ; /* bits 30:30 */
    ca_uint32_t rsrvd1               :  1 ;
  } bf ;
  ca_uint32_t     wrd ;
} F_GLOBAL_FPLL1_t;
#endif /* !__ASSEMBLER__ */



#endif