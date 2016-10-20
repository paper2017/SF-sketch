# The SF-sketch

## Introduction
A **sketch** is a probabilistic data structure that is used to record frequencies of items in a multi-set.
Various types of sketches have been proposed in literature and applied in a variety of fields, such as natural language processing, compressed sensing, network traffic analysis, and distributed data sets.
While several variants of sketches have been proposed in the past, existing sketches still have a significant room for improvement in terms of their memory footprint and accuracy.
Furthermore, existing sketches cannot keep up the performance at an acceptable level due to the rapidly increasing amount of Internet data.
Here, we propose a new sketch, ***the SF-sketch***, that has significantly higher accuracy compared to prior art, while keeping the query speed and update speed unchanged.


## Building

	$ sh build.sh
    
There is an example in `main.c`, which shows the basic usage of ***the SF-sketch***. 
We also implement a series of **slim-fat sketches** (i.e. the SF1, SF2, SF3 and SF4 sketches) that lead to our final ***SFF-sketch***. For example, using `sh build.sh SF1` for our first version of slim-fat sketch (**the SF1-sketch**), we can get executable file: `sf1sketch` in `./bin`, while using `sh build.sh` or `sh build.sh SFF`, we get `sf6sketch` in `./bin`.

NOTICE: CMake 3.1 or higher is required

## Usage

	$ cd ./bin/; ./sffsketch

The command-line shown above is for sffsketch; you can substitute [#] with a number ranging from 1,2,3,4,5,f for any sketch version number in `cd ./bin/; ./sf[#]sketch`.

We should feed two files to `sf[#]sketch`. They are for insertion and deletion operations on items and query for them.
Note that we can apply insertion/deletion many times on a specific item. 
Item's frequency can be defined as # of insertions minus # of deletions.
The default file name for operations is "operations.dat", while "queries.dat" for query. 
In `./workload`, we prepare one example for using those sketches for you, which apply 10K operations on 1K distinct key-value items. 
And "kv1K_op10K.bench" indicating real frequencies for items is shown in the folder `./workload`. 
More complex methods in using `sffsketch` can be found out by `./bin/sffsketch -h` 


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

## Other Sketches
In folder `othersketchs`, we also implement 4 well known sketches (***CMCU sketch, CML sketch, CM sketch and C sketch***) which are used for comparing with our proposed ***SF sketch***. You can build thoses sketches by 
	`cd [skech folder]; ./cmkae .; make`
Usage of thoses sketches is the same as ***SF sketch***.
