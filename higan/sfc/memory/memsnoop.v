// higan / Verilator Co-simulation/emulation model!
// By: defparam
//
// This is a verilog model of a SNES bus snoop/filter.
// This model is instantiated in the SFC core of higan
module memsnoop (
  input              RD_n,
  input              WR_n,
  input      [23:0]  addr,     // SNES Main Bus
  input      [7:0]   data_in,
  output reg [7:0]   data_out
);

initial $display("Verilator memsnoop model loaded!");

/* verilator lint_off UNUSED */
int rnd = 0;
reg [7:0] lastval;
/* verilator lint_on  UNUSED*/

always @(*) begin
  if (~RD_n | ~WR_n) begin // intercept READs/WRITEs, selectively change them
    case(addr)
    /*
      24'h000085: begin // turn on water level physics
        data_out = 8'h01;
      end

      24'h000019: begin // randomly change powerup level
        rnd = $random();
        if ((rnd%50) == 0) begin
          data_out = {6'b0,rnd[31:30]};
          lastval = data_out;
        end
        else begin
          data_out = lastval;
        end
      end
    */
      default: data_out = data_in;
    endcase
  end
end

endmodule

