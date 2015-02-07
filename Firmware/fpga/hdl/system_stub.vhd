-------------------------------------------------------------------------------
-- system_stub.vhd
-------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

library UNISIM;
use UNISIM.VCOMPONENTS.ALL;

entity system_stub is
  port (
    fpga_0_DDR2_SDRAM_DDR2_ODT_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_Addr_pin : out std_logic_vector(12 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_BankAddr_pin : out std_logic_vector(1 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_CAS_n_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_CE_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_CS_n_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_RAS_n_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_WE_n_pin : out std_logic;
    fpga_0_DDR2_SDRAM_DDR2_DM_pin : out std_logic_vector(3 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_DQS : inout std_logic_vector(3 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_DQS_n : inout std_logic_vector(3 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_DQ : inout std_logic_vector(31 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_Clk_pin : out std_logic_vector(1 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_Clk_n_pin : out std_logic_vector(1 downto 0);
    fpga_0_DDR2_SDRAM_DDR2_DQS_Div_I_pin : in std_logic;
    fpga_0_DDR2_SDRAM_DDR2_DQS_Div_O_pin : out std_logic;
    sys_clk_pin : in std_logic;
    sys_rst_pin : in std_logic;
    debug_uart_TX_pin : out std_logic;
    debug_uart_RX_pin : in std_logic;
    sbc_uart_TX_pin : out std_logic;
    sbc_uart_RX_pin : in std_logic;
    sensor_a_spi_MOSI_O_pin : out std_logic;
    sensor_a_spi_MISO_I_pin : in std_logic;
    sensor_a_spi_SCK_O_pin : out std_logic;
    motor_direction_pin : out std_logic;
    motor_step_pin : out std_logic;
    motor_sensor_in_pin : in std_logic_vector(0 to 1);
    motor_channel_sel_pin : out std_logic_vector(0 to 1);
    motor_enable_pin : out std_logic_vector(0 to 2);
    led_spi_MOSI_O_pin : out std_logic;
    led_spi_MISO_I_pin : in std_logic;
    led_spi_SCK_O_pin : out std_logic;
    lcd_data_IO : inout std_logic_vector(0 to 7);
    lcd_rw_pin : out std_logic;
    lcd_rs_pin : out std_logic;
    lcd_enable_pin : out std_logic;
    config_flash_spi_SS_O_pin : out std_logic_vector(0 to 0);
    config_flash_spi_SCK_O_pin : out std_logic;
    config_flash_spi_MOSI_O_pin : out std_logic;
    config_flash_spi_MISO_I_pin : in std_logic;
    user_flash_spi_SS_O_pin : out std_logic_vector(0 to 0);
    user_flash_spi_SCK_O_pin : out std_logic;
    user_flash_spi_MOSI_O_pin : out std_logic;
    user_flash_spi_MISO_I_pin : in std_logic;
    spi_control_sensor_a_addr_pin : out std_logic_vector(0 to 3);
    spi_control_led_channel_pin : out std_logic_vector(0 to 2);
    spi_control_led_spi_addr_pin : out std_logic_vector(0 to 2);
    spi_control_led_photo_sel_pin : out std_logic;
    spi_control_led_fan_enable_pin : out std_logic;
    spi_control_led_enable_pin : out std_logic;
    spi_control_led_analog_control_pin : out std_logic;
    system_dip_sw_pin : in std_logic_vector(0 to 3);
    system_debug_leds_pin : out std_logic_vector(0 to 7);
    system_activity_led_pin : out std_logic;
    system_heartbeat_led_pin : out std_logic;
    system_bootloader_led_pin : out std_logic;
    relay_L_pin : out std_logic;
    relay_B_pin : out std_logic;
    relay_D_pin : out std_logic;
    relay_R_pin : out std_logic;
    relay_F_pin : out std_logic;
    relay_U_pin : out std_logic;
    switch_L_In_Signal_pin : in std_logic;
    switch_L_In_Stimulus_pin : out std_logic;
    switch_L_Out_Signal_pin : in std_logic;
    switch_L_Out_Stimulus_pin : out std_logic;
    switch_F_In_Signal_pin : in std_logic;
    switch_F_In_Stimulus_pin : out std_logic;
    switch_F_Out_Signal_pin : in std_logic;
    switch_F_Out_Stimulus_pin : out std_logic;
    switch_R_In_Signal_pin : in std_logic;
    switch_R_In_Stimulus_pin : out std_logic;
    switch_R_Out_Signal_pin : in std_logic;
    switch_R_Out_Stimulus_pin : out std_logic;
    switch_B_In_Signal_pin : in std_logic;
    switch_B_In_Stimulus_pin : out std_logic;
    switch_B_Out_Signal_pin : in std_logic;
    switch_B_Out_Stimulus_pin : out std_logic;
    switch_U_In_Signal_pin : in std_logic;
    switch_U_In_Stimulus_pin : out std_logic;
    switch_U_Out_Signal_pin : in std_logic;
    switch_U_Out_Stimulus_pin : out std_logic;
    switch_D_In_Signal_pin : in std_logic;
    switch_D_In_Stimulus_pin : out std_logic;
    switch_D_Out_Signal_pin : in std_logic;
    switch_D_Out_Stimulus_pin : out std_logic;
    motor2_direction_pin : out std_logic;
    motor2_step_pin : out std_logic;
    motor2_sensor_in_pin : in std_logic_vector(0 to 1);
    motor2_channel_sel_pin : out std_logic_vector(0 to 1);
    motor2_enable_pin : out std_logic_vector(0 to 2)
  );
end system_stub;

architecture STRUCTURE of system_stub is

  component system is
    port (
      fpga_0_DDR2_SDRAM_DDR2_ODT_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_Addr_pin : out std_logic_vector(12 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_BankAddr_pin : out std_logic_vector(1 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_CAS_n_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_CE_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_CS_n_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_RAS_n_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_WE_n_pin : out std_logic;
      fpga_0_DDR2_SDRAM_DDR2_DM_pin : out std_logic_vector(3 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_DQS : inout std_logic_vector(3 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_DQS_n : inout std_logic_vector(3 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_DQ : inout std_logic_vector(31 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_Clk_pin : out std_logic_vector(1 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_Clk_n_pin : out std_logic_vector(1 downto 0);
      fpga_0_DDR2_SDRAM_DDR2_DQS_Div_I_pin : in std_logic;
      fpga_0_DDR2_SDRAM_DDR2_DQS_Div_O_pin : out std_logic;
      sys_clk_pin : in std_logic;
      sys_rst_pin : in std_logic;
      debug_uart_TX_pin : out std_logic;
      debug_uart_RX_pin : in std_logic;
      sbc_uart_TX_pin : out std_logic;
      sbc_uart_RX_pin : in std_logic;
      sensor_a_spi_MOSI_O_pin : out std_logic;
      sensor_a_spi_MISO_I_pin : in std_logic;
      sensor_a_spi_SCK_O_pin : out std_logic;
      motor_direction_pin : out std_logic;
      motor_step_pin : out std_logic;
      motor_sensor_in_pin : in std_logic_vector(0 to 1);
      motor_channel_sel_pin : out std_logic_vector(0 to 1);
      motor_enable_pin : out std_logic_vector(0 to 2);
      led_spi_MOSI_O_pin : out std_logic;
      led_spi_MISO_I_pin : in std_logic;
      led_spi_SCK_O_pin : out std_logic;
      lcd_data_IO : inout std_logic_vector(0 to 7);
      lcd_rw_pin : out std_logic;
      lcd_rs_pin : out std_logic;
      lcd_enable_pin : out std_logic;
      config_flash_spi_SS_O_pin : out std_logic_vector(0 to 0);
      config_flash_spi_SCK_O_pin : out std_logic;
      config_flash_spi_MOSI_O_pin : out std_logic;
      config_flash_spi_MISO_I_pin : in std_logic;
      user_flash_spi_SS_O_pin : out std_logic_vector(0 to 0);
      user_flash_spi_SCK_O_pin : out std_logic;
      user_flash_spi_MOSI_O_pin : out std_logic;
      user_flash_spi_MISO_I_pin : in std_logic;
      spi_control_sensor_a_addr_pin : out std_logic_vector(0 to 3);
      spi_control_led_channel_pin : out std_logic_vector(0 to 2);
      spi_control_led_spi_addr_pin : out std_logic_vector(0 to 2);
      spi_control_led_photo_sel_pin : out std_logic;
      spi_control_led_fan_enable_pin : out std_logic;
      spi_control_led_enable_pin : out std_logic;
      spi_control_led_analog_control_pin : out std_logic;
      system_dip_sw_pin : in std_logic_vector(0 to 3);
      system_debug_leds_pin : out std_logic_vector(0 to 7);
      system_activity_led_pin : out std_logic;
      system_heartbeat_led_pin : out std_logic;
      system_bootloader_led_pin : out std_logic;
      relay_L_pin : out std_logic;
      relay_B_pin : out std_logic;
      relay_D_pin : out std_logic;
      relay_R_pin : out std_logic;
      relay_F_pin : out std_logic;
      relay_U_pin : out std_logic;
      switch_L_In_Signal_pin : in std_logic;
      switch_L_In_Stimulus_pin : out std_logic;
      switch_L_Out_Signal_pin : in std_logic;
      switch_L_Out_Stimulus_pin : out std_logic;
      switch_F_In_Signal_pin : in std_logic;
      switch_F_In_Stimulus_pin : out std_logic;
      switch_F_Out_Signal_pin : in std_logic;
      switch_F_Out_Stimulus_pin : out std_logic;
      switch_R_In_Signal_pin : in std_logic;
      switch_R_In_Stimulus_pin : out std_logic;
      switch_R_Out_Signal_pin : in std_logic;
      switch_R_Out_Stimulus_pin : out std_logic;
      switch_B_In_Signal_pin : in std_logic;
      switch_B_In_Stimulus_pin : out std_logic;
      switch_B_Out_Signal_pin : in std_logic;
      switch_B_Out_Stimulus_pin : out std_logic;
      switch_U_In_Signal_pin : in std_logic;
      switch_U_In_Stimulus_pin : out std_logic;
      switch_U_Out_Signal_pin : in std_logic;
      switch_U_Out_Stimulus_pin : out std_logic;
      switch_D_In_Signal_pin : in std_logic;
      switch_D_In_Stimulus_pin : out std_logic;
      switch_D_Out_Signal_pin : in std_logic;
      switch_D_Out_Stimulus_pin : out std_logic;
      motor2_direction_pin : out std_logic;
      motor2_step_pin : out std_logic;
      motor2_sensor_in_pin : in std_logic_vector(0 to 1);
      motor2_channel_sel_pin : out std_logic_vector(0 to 1);
      motor2_enable_pin : out std_logic_vector(0 to 2)
    );
  end component;

begin

  system_i : system
    port map (
      fpga_0_DDR2_SDRAM_DDR2_ODT_pin => fpga_0_DDR2_SDRAM_DDR2_ODT_pin,
      fpga_0_DDR2_SDRAM_DDR2_Addr_pin => fpga_0_DDR2_SDRAM_DDR2_Addr_pin,
      fpga_0_DDR2_SDRAM_DDR2_BankAddr_pin => fpga_0_DDR2_SDRAM_DDR2_BankAddr_pin,
      fpga_0_DDR2_SDRAM_DDR2_CAS_n_pin => fpga_0_DDR2_SDRAM_DDR2_CAS_n_pin,
      fpga_0_DDR2_SDRAM_DDR2_CE_pin => fpga_0_DDR2_SDRAM_DDR2_CE_pin,
      fpga_0_DDR2_SDRAM_DDR2_CS_n_pin => fpga_0_DDR2_SDRAM_DDR2_CS_n_pin,
      fpga_0_DDR2_SDRAM_DDR2_RAS_n_pin => fpga_0_DDR2_SDRAM_DDR2_RAS_n_pin,
      fpga_0_DDR2_SDRAM_DDR2_WE_n_pin => fpga_0_DDR2_SDRAM_DDR2_WE_n_pin,
      fpga_0_DDR2_SDRAM_DDR2_DM_pin => fpga_0_DDR2_SDRAM_DDR2_DM_pin,
      fpga_0_DDR2_SDRAM_DDR2_DQS => fpga_0_DDR2_SDRAM_DDR2_DQS,
      fpga_0_DDR2_SDRAM_DDR2_DQS_n => fpga_0_DDR2_SDRAM_DDR2_DQS_n,
      fpga_0_DDR2_SDRAM_DDR2_DQ => fpga_0_DDR2_SDRAM_DDR2_DQ,
      fpga_0_DDR2_SDRAM_DDR2_Clk_pin => fpga_0_DDR2_SDRAM_DDR2_Clk_pin,
      fpga_0_DDR2_SDRAM_DDR2_Clk_n_pin => fpga_0_DDR2_SDRAM_DDR2_Clk_n_pin,
      fpga_0_DDR2_SDRAM_DDR2_DQS_Div_I_pin => fpga_0_DDR2_SDRAM_DDR2_DQS_Div_I_pin,
      fpga_0_DDR2_SDRAM_DDR2_DQS_Div_O_pin => fpga_0_DDR2_SDRAM_DDR2_DQS_Div_O_pin,
      sys_clk_pin => sys_clk_pin,
      sys_rst_pin => sys_rst_pin,
      debug_uart_TX_pin => debug_uart_TX_pin,
      debug_uart_RX_pin => debug_uart_RX_pin,
      sbc_uart_TX_pin => sbc_uart_TX_pin,
      sbc_uart_RX_pin => sbc_uart_RX_pin,
      sensor_a_spi_MOSI_O_pin => sensor_a_spi_MOSI_O_pin,
      sensor_a_spi_MISO_I_pin => sensor_a_spi_MISO_I_pin,
      sensor_a_spi_SCK_O_pin => sensor_a_spi_SCK_O_pin,
      motor_direction_pin => motor_direction_pin,
      motor_step_pin => motor_step_pin,
      motor_sensor_in_pin => motor_sensor_in_pin,
      motor_channel_sel_pin => motor_channel_sel_pin,
      motor_enable_pin => motor_enable_pin,
      led_spi_MOSI_O_pin => led_spi_MOSI_O_pin,
      led_spi_MISO_I_pin => led_spi_MISO_I_pin,
      led_spi_SCK_O_pin => led_spi_SCK_O_pin,
      lcd_data_IO => lcd_data_IO,
      lcd_rw_pin => lcd_rw_pin,
      lcd_rs_pin => lcd_rs_pin,
      lcd_enable_pin => lcd_enable_pin,
      config_flash_spi_SS_O_pin => config_flash_spi_SS_O_pin(0 to 0),
      config_flash_spi_SCK_O_pin => config_flash_spi_SCK_O_pin,
      config_flash_spi_MOSI_O_pin => config_flash_spi_MOSI_O_pin,
      config_flash_spi_MISO_I_pin => config_flash_spi_MISO_I_pin,
      user_flash_spi_SS_O_pin => user_flash_spi_SS_O_pin(0 to 0),
      user_flash_spi_SCK_O_pin => user_flash_spi_SCK_O_pin,
      user_flash_spi_MOSI_O_pin => user_flash_spi_MOSI_O_pin,
      user_flash_spi_MISO_I_pin => user_flash_spi_MISO_I_pin,
      spi_control_sensor_a_addr_pin => spi_control_sensor_a_addr_pin,
      spi_control_led_channel_pin => spi_control_led_channel_pin,
      spi_control_led_spi_addr_pin => spi_control_led_spi_addr_pin,
      spi_control_led_photo_sel_pin => spi_control_led_photo_sel_pin,
      spi_control_led_fan_enable_pin => spi_control_led_fan_enable_pin,
      spi_control_led_enable_pin => spi_control_led_enable_pin,
      spi_control_led_analog_control_pin => spi_control_led_analog_control_pin,
      system_dip_sw_pin => system_dip_sw_pin,
      system_debug_leds_pin => system_debug_leds_pin,
      system_activity_led_pin => system_activity_led_pin,
      system_heartbeat_led_pin => system_heartbeat_led_pin,
      system_bootloader_led_pin => system_bootloader_led_pin,
      relay_L_pin => relay_L_pin,
      relay_B_pin => relay_B_pin,
      relay_D_pin => relay_D_pin,
      relay_R_pin => relay_R_pin,
      relay_F_pin => relay_F_pin,
      relay_U_pin => relay_U_pin,
      switch_L_In_Signal_pin => switch_L_In_Signal_pin,
      switch_L_In_Stimulus_pin => switch_L_In_Stimulus_pin,
      switch_L_Out_Signal_pin => switch_L_Out_Signal_pin,
      switch_L_Out_Stimulus_pin => switch_L_Out_Stimulus_pin,
      switch_F_In_Signal_pin => switch_F_In_Signal_pin,
      switch_F_In_Stimulus_pin => switch_F_In_Stimulus_pin,
      switch_F_Out_Signal_pin => switch_F_Out_Signal_pin,
      switch_F_Out_Stimulus_pin => switch_F_Out_Stimulus_pin,
      switch_R_In_Signal_pin => switch_R_In_Signal_pin,
      switch_R_In_Stimulus_pin => switch_R_In_Stimulus_pin,
      switch_R_Out_Signal_pin => switch_R_Out_Signal_pin,
      switch_R_Out_Stimulus_pin => switch_R_Out_Stimulus_pin,
      switch_B_In_Signal_pin => switch_B_In_Signal_pin,
      switch_B_In_Stimulus_pin => switch_B_In_Stimulus_pin,
      switch_B_Out_Signal_pin => switch_B_Out_Signal_pin,
      switch_B_Out_Stimulus_pin => switch_B_Out_Stimulus_pin,
      switch_U_In_Signal_pin => switch_U_In_Signal_pin,
      switch_U_In_Stimulus_pin => switch_U_In_Stimulus_pin,
      switch_U_Out_Signal_pin => switch_U_Out_Signal_pin,
      switch_U_Out_Stimulus_pin => switch_U_Out_Stimulus_pin,
      switch_D_In_Signal_pin => switch_D_In_Signal_pin,
      switch_D_In_Stimulus_pin => switch_D_In_Stimulus_pin,
      switch_D_Out_Signal_pin => switch_D_Out_Signal_pin,
      switch_D_Out_Stimulus_pin => switch_D_Out_Stimulus_pin,
      motor2_direction_pin => motor2_direction_pin,
      motor2_step_pin => motor2_step_pin,
      motor2_sensor_in_pin => motor2_sensor_in_pin,
      motor2_channel_sel_pin => motor2_channel_sel_pin,
      motor2_enable_pin => motor2_enable_pin
    );

end architecture STRUCTURE;

