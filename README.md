# The SF-sketch
#
## Introduction
A **sketch** is a probabilistic data structure used to record frequencies of items in a multi-set.
Sketches are widely used in various fields, especially those that involve processing and storing data streams.
In streaming applications with high data rates, a sketch "fills up" very quickly.
Thus, its contents are periodically transferred to a remote collector, which is responsible for answering queries.
In this paper, we propose a new sketch, called Slim-Fat (SF) sketch, which has a significantly higher accuracy compared to prior art and at the same time achieves comparable speed as the best prior sketch.
The key idea behind our proposed SF-sketch is to maintain two separate sketches: a large sketch called Fat-subsketch and a small sketch called Slim-subsketch.
The Fat-subsketch is used to perform updates, and periodically produce the Slim-subsketch, which is periodically transferred to the remote collector for answering queries quickly and accurately.
For the final version of our SF-sketch, we derived the error bound and an accurate correct rate formula verified by experiments.
We implemented and extensively evaluated SF-sketch along with several prior sketches and compared them side by side.
Our experimental results show that SF-sketch outperforms the most widely used CM-sketch by up to 33.1 times in terms of accuracy.


## Building
We implement ***SF-sketch*** and 4 well known sketches (***CMCU sketch, A sketch, CM sketch and C sketch***) which are used for comparing with our proposed ***SF sketch***. You can build thoses sketches by 

	cd [sketch folder]; ./cmkae .; make
	  
There is an example in `main.c`, which shows the basic usage of the corresponding sketch. 
Type `[sketch folder]/bin/[sketch] -h` to show more information.

NOTICE: CMake 3.1 or higher is required

## Usage

	$ cd ./bench_sketch/; make;
	$ sh ./experiment_on_timming.sh
	$ sh ./experiment_on_aging.sh


## Workloads Generation
To test the performance of sketches in different scenarios, we harness [YCSB](https://github.com/brianfrankcooper/YCSB.git) to generate two kinds of workloads: **uniform** and **skewed ([zipfian](https://en.wikipedia.org/wiki/Zipf%27s_law))**.
We also use [memcached](https://github.com/memcached/memcached.git) and the generated workloads to record the real frequency of each item as benchmarks.
After that, we feed the generated workloads to our proposed SF-sketch to record the estimation of itme frequency using the SF-sketch.
Then, we calculate the average relative error and empirical cumulative distribution function (CDF) of relative error using benchmarks and estimations.
To compare the accuracy with other well known sketches (such as Count-min (CM) sketch, conservative update (CU) sketch, etc.), we can use the same method to get the average relative error and CDF.

Run YCSB command

	bin/ycsb load basic -P workloads/workloadc -p fieldcount=1 -p fieldlength=32 -p recordcount=100000 -p operationcount=10000000 -p requestdistribution=uniform > workload/kv100K_op10M_uniform_init.raw
	bin/ycsb run basic -P workloads/workloadc -p fieldcount=1 -p fieldlength=32 -p recordcount=100000 -p operationcount=10000000 -p requestdistribution=uniform > workload/kv100K_op10M_uniform_insert.raw

to get a uniform distributed workload with 10M operations on the 100K distinct items. Changing `requestdistribution=uniform` to `requestdistribution=zipfian`, we can get a zipfian distributed workload.


