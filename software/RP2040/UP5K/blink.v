
module blink (input  clk,
              output led_blue,
              output led_green,
              output led_red,
              input  lsd_adc_ofl,
              input  lsd_adc_d7,
              input  lsd_adc_d6,
              input  lsd_adc_d5,
              input  lsd_adc_d4,
              input  lsd_adc_d3,
              input  lsd_adc_d2,
              input  lsd_adc_d1,
              input  lsd_adc_d0,
              input  lsd_adc_int,
              output lsd_adc_cs,
              output lsd_adc_rd,
              output lsd_adc_mode,
              output lsd_adc_wr_rdy,
              input  gps_pps1,
              input  gps_pps2,
              input  sense_precise,
              output data_hv,
              output clock_hv,
              output data_precise,
              output clock_precise,
              output pps_trimmed,
              inout  i2c_data,
              inout  i2c_clock);


//-- Modify this value for changing the blink frequency
localparam N = 25;  //-- N<=21 Fast, N>=23 Slow

reg [N:0] counter;
always @(posedge clk)
  counter <= counter + 1;

assign led_green = counter[N];
assign led_blue = counter[N-1];
assign led_red = counter[N-2];

endmodule
