# cache_sim

2018 2학기 컴퓨터 구조 수업
n-way set associativity cache simulator 구현하고 Block size, Cache size, Associativity, Replacement Policy(lru, rand)에 따라 cache miss rate 값을 비교 분석하는 텀 프로젝트

---
## How to run
```bash
# clone repository
$ git clone https://github.com/atg0831/cache_sim.git
$ cd cache_sim

# compile first
$ gcc cache_sim.co -o cache_sim

# run with argv
$ ./cache_sim -s $cache_size -b $block_size -a $associativity -r $replacement_policy -f $trace_file_name

# Or you just run without argv
# default - cache_size :1024, block_size: 32, associativiy: 16, replacement_policy: rand, trace_file_name: memtrace.trc
$ ./cache_sim
