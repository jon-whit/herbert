// This file has been automatically generated.  Do not modify.
parameter C_TWR                  = 15000;
parameter C_CTRL_AP_COL_DELAY = 4'h1;
parameter C_CTRL_AP_PI_ADDR_CE_DELAY = 4'h0;
parameter C_CTRL_AP_PORT_SELECT_DELAY = 4'h0;
parameter C_CTRL_AP_PIPELINE1_CE_DELAY = 4'h0;
parameter C_CTRL_DP_LOAD_RDWDADDR_DELAY = 4'h2;
parameter C_CTRL_DP_RDFIFO_WHICHPORT_DELAY = 4'hc;
parameter C_CTRL_DP_SIZE_DELAY = 4'h2;
parameter C_CTRL_DP_WRFIFO_WHICHPORT_DELAY = 4'h4;
parameter C_CTRL_PHYIF_DUMMYREADSTART_DELAY = 4'h5;
parameter C_CTRL_Q0_DELAY = 4'h1;
parameter C_CTRL_Q1_DELAY = 4'h0;
parameter C_CTRL_Q2_DELAY = 4'h2;
parameter C_CTRL_Q3_DELAY = 4'h2;
parameter C_CTRL_Q4_DELAY = 4'h2;
parameter C_CTRL_Q5_DELAY = 4'h2;
parameter C_CTRL_Q6_DELAY = 4'h5;
parameter C_CTRL_Q7_DELAY = 4'h2;
parameter C_CTRL_Q8_DELAY = 4'h1;
parameter C_CTRL_Q9_DELAY = 4'h2;
parameter C_CTRL_Q10_DELAY = 4'h1;
parameter C_CTRL_Q11_DELAY = 4'h2;
parameter C_CTRL_Q12_DELAY = 4'h1;
parameter C_CTRL_Q13_DELAY = 4'h1;
parameter C_CTRL_Q14_DELAY = 4'h1;
parameter C_CTRL_Q15_DELAY = 4'h1;
parameter C_CTRL_Q16_DELAY = 4'h1;
parameter C_CTRL_Q17_DELAY = 4'h0;
parameter C_CTRL_Q18_DELAY = 4'h0;
parameter C_CTRL_Q19_DELAY = 4'h0;
parameter C_CTRL_Q20_DELAY = 4'h0;
parameter C_CTRL_Q21_DELAY = 4'h0;
parameter C_CTRL_Q22_DELAY = 4'h0;
parameter C_CTRL_Q23_DELAY = 4'h0;
parameter C_CTRL_Q24_DELAY = 4'h0;
parameter C_CTRL_Q25_DELAY = 4'h0;
parameter C_CTRL_Q26_DELAY = 4'h0;
parameter C_CTRL_Q27_DELAY = 4'h0;
parameter C_CTRL_Q28_DELAY = 4'h0;
parameter C_CTRL_Q29_DELAY = 4'h0;
parameter C_CTRL_Q30_DELAY = 4'h0;
parameter C_CTRL_Q31_DELAY = 4'h0;
parameter C_CTRL_Q32_DELAY = 4'h2;
parameter C_CTRL_Q33_DELAY = 4'h1;
parameter C_CTRL_Q34_DELAY = 4'h0;
parameter C_CTRL_Q35_DELAY = 4'h0;
parameter C_BASEADDR_CTRL0 = 9'h000;
parameter C_HIGHADDR_CTRL0 = 9'h00e;
parameter integer C_SKIP_1_VALUE = 9'h001;
parameter C_BASEADDR_CTRL1 = 9'h00f;
parameter C_HIGHADDR_CTRL1 = 9'h018;
parameter C_BASEADDR_CTRL2 = 9'h019;
parameter C_HIGHADDR_CTRL2 = 9'h027;
parameter integer C_SKIP_2_VALUE = 9'h001;
parameter C_BASEADDR_CTRL3 = 9'h028;
parameter C_HIGHADDR_CTRL3 = 9'h031;
parameter C_BASEADDR_CTRL4 = 9'h032;
parameter C_HIGHADDR_CTRL4 = 9'h040;
parameter integer C_SKIP_3_VALUE = 9'h001;
parameter C_BASEADDR_CTRL5 = 9'h041;
parameter C_HIGHADDR_CTRL5 = 9'h04a;
parameter C_BASEADDR_CTRL6 = 9'h04b;
parameter C_HIGHADDR_CTRL6 = 9'h05b;
parameter integer C_SKIP_4_VALUE = 9'h001;
parameter C_BASEADDR_CTRL7 = 9'h05c;
parameter C_HIGHADDR_CTRL7 = 9'h066;
parameter C_B16_REPEAT_CNT = 0;
parameter C_BASEADDR_CTRL8 = 9'h067;
parameter C_HIGHADDR_CTRL8 = 9'h07b;
parameter integer C_SKIP_5_VALUE = 9'h001;
parameter C_BASEADDR_CTRL9 = 9'h07c;
parameter C_HIGHADDR_CTRL9 = 9'h08a;
parameter C_B32_REPEAT_CNT = 2;
parameter C_BASEADDR_CTRL10 = 9'h08b;
parameter C_HIGHADDR_CTRL10 = 9'h09f;
parameter integer C_SKIP_6_VALUE = 9'h001;
parameter C_BASEADDR_CTRL11 = 9'h0a0;
parameter C_HIGHADDR_CTRL11 = 9'h0ae;
parameter C_B64_REPEAT_CNT = 6;
parameter C_BASEADDR_CTRL12 = 9'h0af;
parameter C_HIGHADDR_CTRL12 = 9'h0c3;
parameter integer C_SKIP_7_VALUE = 9'h001;
parameter C_BASEADDR_CTRL13 = 9'h0c4;
parameter C_HIGHADDR_CTRL13 = 9'h0d2;
parameter C_BASEADDR_CTRL14 = 9'h0d3;
parameter C_HIGHADDR_CTRL14 = 9'h0e1;
parameter C_BASEADDR_CTRL15 = 9'h0e2;
parameter C_HIGHADDR_CTRL15 = 9'h0e3;
parameter C_CTRL_BRAM_INIT_3F = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_3E = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_3D = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_3C = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_3B = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_3A = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_39 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_38 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_37 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_36 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_35 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_34 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_33 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_32 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_31 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_30 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2F = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2E = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2D = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2C = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2B = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_2A = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_29 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_28 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_27 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_26 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_25 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_24 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_23 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_22 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_21 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_20 = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_1F = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_1E = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_1D = 256'h000002FC000002FC000002FC000002FC000002FC000002FC000002FC000002FC;
parameter C_CTRL_BRAM_INIT_1C = 256'h000002FC000002FC000002FC000002FC0000003C0000003C0000003C0000003C;
parameter C_CTRL_BRAM_INIT_1B = 256'h0000003C0000003C0000003C0000003D0000003C0000003C0000003C0000003C;
parameter C_CTRL_BRAM_INIT_1A = 256'h000000300000003C0000003C0000003C000040280000003C0000003C0000003C;
parameter C_CTRL_BRAM_INIT_19 = 256'h000040280000003C000004350000243C000004340000243C000004340000243C;
parameter C_CTRL_BRAM_INIT_18 = 256'h000004340000943C0000803C000080380001003C0001003C0001003C0001003C;
parameter C_CTRL_BRAM_INIT_17 = 256'h000140280001003D0001003C0001003C0001003C0001003C0001003C00010024;
parameter C_CTRL_BRAM_INIT_16 = 256'h0001213C000101240001213C000101240001213C000101240001913C0001813C;
parameter C_CTRL_BRAM_INIT_15 = 256'h000180380000003C0000003C0000003C000040280000003C000004350000243C;
parameter C_CTRL_BRAM_INIT_14 = 256'h000004340000243C000004340000243C000004340000943C0000803C00008038;
parameter C_CTRL_BRAM_INIT_13 = 256'h0001003C0001003C0001003C0001003C000140280001003D0001003C0001003C;
parameter C_CTRL_BRAM_INIT_12 = 256'h0001003C0001003C0001003C000100240001213C000101240001213C00010124;
parameter C_CTRL_BRAM_INIT_11 = 256'h0001213C000101240001913C0001813C000180380000003C0000003C0000003C;
parameter C_CTRL_BRAM_INIT_10 = 256'h000040280000003C000004350000243C000004340000243C000004340000243C;
parameter C_CTRL_BRAM_INIT_0F = 256'h000004340000943C0000803C000080380001003C0001003C0001003C0001003C;
parameter C_CTRL_BRAM_INIT_0E = 256'h000140280001003D0001003C0001003C0001003C0001003C0001003C00010024;
parameter C_CTRL_BRAM_INIT_0D = 256'h0001213C000101240001213C000101240001213C000101240001913C0001813C;
parameter C_CTRL_BRAM_INIT_0C = 256'h000180380000003C0000003C0000003C000040280000003C000004350000243C;
parameter C_CTRL_BRAM_INIT_0B = 256'h000004340000943C0000803C000080380001003C0001003C0001003C0001003C;
parameter C_CTRL_BRAM_INIT_0A = 256'h000140280001003D0001003C0001003C0001003C0001003C0001003C00010024;
parameter C_CTRL_BRAM_INIT_09 = 256'h0001213C000101240001913C0001813C000180380000003C0000003C0000003C;
parameter C_CTRL_BRAM_INIT_08 = 256'h000040280000003C0000003D000004340000943C0000803C000080380001003C;
parameter C_CTRL_BRAM_INIT_07 = 256'h0001003C0001003C0001003C000140280001003D0001003C0001003C0001003C;
parameter C_CTRL_BRAM_INIT_06 = 256'h0001003C0001003C000100240001913C0001813C000180380000003C0000003C;
parameter C_CTRL_BRAM_INIT_05 = 256'h0000003C000040280000003C0000003D000000340000943C0000803C00008038;
parameter C_CTRL_BRAM_INIT_04 = 256'h0001003C0001003C0001003C0001003C000140280001003D0001003C0001003C;
parameter C_CTRL_BRAM_INIT_03 = 256'h0001003C0001003C0001003C000100240001913C0001813C000180380000003C;
parameter C_CTRL_BRAM_INIT_02 = 256'h0000003C0000003C000040280000003C0000003D000000340000943C0000803C;
parameter C_CTRL_BRAM_INIT_01 = 256'h000080380001003C0001003C0001003C0001003C000140280001003D0001003C;
parameter C_CTRL_BRAM_INIT_00 = 256'h0001003C0001003C0001003C0001003C000100240001913C0001813C00018038;
parameter C_CTRL_BRAM_SRVAL = 36'h0000002FC;
parameter C_CTRL_BRAM_INITP_07 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter C_CTRL_BRAM_INITP_06 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter C_CTRL_BRAM_INITP_05 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter C_CTRL_BRAM_INITP_04 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
parameter C_CTRL_BRAM_INITP_03 = 256'h0000000000000000000000000000000000000000000000000000000200000000;
parameter C_CTRL_BRAM_INITP_02 = 256'h0001111100002000000000000002000000000001111100002000000000000000;
parameter C_CTRL_BRAM_INITP_01 = 256'h0000000000011111000000000000000000000000000111110000000000000000;
parameter C_CTRL_BRAM_INITP_00 = 256'h0000001111100000000000000000000111111000000000000000000011111100;
parameter C_MEM_CAS_LATENCY0 = 4'd3;
