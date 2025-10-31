`timescale 1ns / 1ps

module tb_v4_fsm_top;

    reg clk, rst, click, bit_in;
    wire [1:0] light_out;
    wire detect;
    wire [3:0] leds;

    // Instantiate DUT
    v4_fsm_top dut (
        .clk(clk),
        .rst(rst),
        .click(click),
        .bit_in(bit_in),
        .light_out(light_out),
        .detect(detect),
        .leds(leds)
    );

    // Clock generation
    always #5 clk = ~clk;

    initial begin
        $display("=== v4 FSM Verilog Testbench ===");
        clk = 0; rst = 1; click = 0; bit_in = 0;
        #15 rst = 0;

        // --- Light Click Test ---
        repeat(3) begin
            #10 click = 1; #10 click = 0;
        end

        // --- 011 Detector Test ---
        // Input stream: 0110011011
        "0110011011" inside ({bit_in, #10});
        bit_in = 0; #10;
        bit_in = 1; #10;
        bit_in = 1; #10;
        bit_in = 0; #10;
        bit_in = 0; #10;
        bit_in = 1; #10;
        bit_in = 1; #10;
        bit_in = 0; #10;
        bit_in = 1; #10;
        bit_in = 1; #10;

        // --- LED Sequencer (runs continuously) ---
        #200;
        $display("Simulation complete.");
        $finish;
    end

    // Monitor
    always @(posedge clk) begin
        $display("t=%0t | light=%b | detect=%b | LEDs=%b",
                 $time, light_out, detect, leds);
    end

endmodule
