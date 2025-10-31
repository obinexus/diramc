`timescale 1ns / 1ps
//====================================================================//
//  v4 Mealy/Moore FSM Demo – Verilog HDL
//  -------------------------------------
//  • Light Click FSM (3-state brightness)
//  • 011 Sequence Detector (overlapping)
//  • 4-LED Sequencer (1000 → 0100 → 0010 → 0001)
//  • Moore machine (output = f(state))
//  • Synthesizable, reset-synchronous
//====================================================================//

module v4_fsm_top (
    input  wire       clk,        // System clock
    input  wire       rst,        // Active-high reset
    input  wire       click,      // Light click input
    input  wire       bit_in,     // Serial bit for 011 detector
    output reg  [1:0] light_out,  // 00=off, 01=dim, 10=bright
    output reg        detect,     // High when "011" found
    output reg  [3:0] leds        // LED sequencer output
);

//====================================================================//
// 1. Light Click FSM (Moore)
//====================================================================//
reg [1:0] light_state;  // 00=OFF, 01=DIM, 10=BRIGHT

always @(posedge clk) begin
    if (rst)
        light_state <= 2'b00;
    else begin
        if (click) begin
            light_state <= (light_state == 2'b10) ? 2'b00 : light_state + 1;
        end
        // else stay
    end
end

always @(*) begin
    light_out = light_state;  // Moore: output = state
end

//====================================================================//
// 2. 011 Sequence Detector (Moore, overlapping)
//====================================================================//
reg [1:0] det_state;  // S0=00, S1=01, S2=10, S3=11

always @(posedge clk) begin
    if (rst)
        det_state <= 2'b00;
    else begin
        case (det_state)
            2'b00: det_state <= bit_in ? 2'b01 : 2'b00;  // S0
            2'b01: det_state <= bit_in ? 2'b10 : 2'b00;  // S1
            2'b10: det_state <= bit_in ? 2'b10 : 2'b11;  // S2
            2'b11: det_state <= bit_in ? 2'b01 : 2'b00;  // S3 → restart
        endcase
    end
end

always @(*) begin
    detect = (det_state == 2'b11);  // S3 = "011" found
end

//====================================================================//
// 3. LED Sequencer (Moore)
//====================================================================//
reg [1:0] led_state;  // 00=1000, 01=0100, 10=0010, 11=0001

always @(posedge clk) begin
    if (rst)
        led_state <= 2'b00;
    else begin
        led_state <= led_state + 1;  // auto-wrap at 11 → 00
    end
end

always @(*) begin
    case (led_state)
        2'b00: leds = 4'b1000;
        2'b01: leds = 4'b0100;
        2'b10: leds = 4'b0010;
        2'b11: leds = 4'b0001;
        default: leds = 4'b0000;
    endcase
end

endmodule
