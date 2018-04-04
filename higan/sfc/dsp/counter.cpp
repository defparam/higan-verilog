//counter_rate = number of samples per counter event
//all rates are evenly divisible by counter_range (0x7800, 30720, or 2048 * 5 * 3)
//note that rate[0] is a special case, which never triggers

const uint16 DSP::CounterRate[32] = {
     0, 2048, 1536,
  1280, 1024,  768,
   640,  512,  384,
   320,  256,  192,
   160,  128,   96,
    80,   64,   48,
    40,   32,   24,
    20,   16,   12,
    10,    8,    6,
     5,    4,    3,
           2,
           1,
};

//counter_offset = counter offset from zero
//counters do not appear to be aligned at zero for all rates

const uint16 DSP::CounterOffset[32] = {
    0, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
  536, 0, 1040,
       0,
       0,
};

inline auto DSP::counterTick() -> void {
  state.counter--;
  if(state.counter < 0) state.counter = CounterRange - 1;
}

//return true if counter event should trigger

inline auto DSP::counterPoll(uint rate) -> bool {
  if(rate == 0) return false;
  return (((uint)state.counter + CounterOffset[rate]) % CounterRate[rate]) == 0;
}
