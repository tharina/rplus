# path to sqlplot-tools
SQLPLOTS ?= $(shell which sp-process >/dev/null && echo sp-process || echo `pwd`/sp-process)

# default set of benchmarks for which plots should be generated
BENCHMARKS ?= uniform skewed normal clustered stacked
# default set of plot types that should be generated
PLOTS ?= pretime prealloc time l1dcm l3tcm brmsp totins totcyc
# datastructures that have been evaluated
DATASTRUCTURES ?= $(shell grep -oE 'ds=[^	]+' ../results/*.txt | sed -E 's/^.+ds=//' | sort -u | sed 's/ /_/g')
# set to non-empty to keep intermediate files
KEEP_PLOTS ?=

# all known plot types
PLOT_TYPES := time l1dcm l3tcm brmsp totins totcyc alloc peakmem nalloc
PLOT_TYPES += $(patsubst %,pre%,$(PLOT_TYPES))

# strips non-alphanumeric characters from datastructure names
ds_target = $(shell echo $1 | sed 's/[^a-zA-Z0-9]/_/g')
# maps plot type to results file
dep_file  = ../results/$(if $(findstring time,$1),time,$(if $(findstring alloc,$1),memory,$(if $(findstring peakmem,$1),memory,papi))).txt

# datastructure targets
DS_TARGETS := $(foreach d,$(DATASTRUCTURES),$(call ds_target,$d))

# default target. if there is only one datastructure, generate plots comparing benchmarks;
# otherwise, generate one plot for each benchmark comparing datastructures
all: $(if $(findstring 1,$(words $(DATASTRUCTURES))),ds,bench)

# make plots for each benchmark
bench: $(BENCHMARKS)

# make plots for each datastructure
ds: $(DS_TARGETS)

# file name stems, of the from <plot type>_<benchmark or datastructure>
STEMS := $(foreach p,$(PLOT_TYPES),$(foreach b,$(BENCHMARKS),$p_$b) $(foreach d,$(DS_TARGETS),$p_$d))

# make absolutely everything
everything: $(foreach s,$(STEMS),$s.svg)

# clean up all plots
clean:
	@rm -f $(STEMS:=.plot) $(STEMS:=-data.txt) $(STEMS:=.svg)

.PHONY: all ds bench clean $(PLOT_TYPES) $(BENCHMARKS) $(foreach d,$(DATASTRUCTURES),$(call ds_target,$d))
# delete intermediate plot files, but keep those specified as target on the command line
.INTERMEDIATE: $(if $(KEEP_PLOTS),,$(filter-out $(foreach g,$(filter %.plot,$(MAKECMDGOALS)),$g $(g:.plot=-data.txt)),$(STEMS:=.plot) $(STEMS:=-data.txt)))
.SUFFIXES:

# sets up dependencies for plot files and sets column names
define plot-deps
$1: $(foreach b,$(BENCHMARKS),$1_$b.svg)
$(foreach b,$(BENCHMARKS),$1_$b.plot): $(call dep_file,$1)
$(foreach d,$(DS_TARGETS),$1_$d.plot): $(call dep_file,$1)
$1_%.plot: COLUMN := $1
# $(foreach b,$(BENCHMARKS),$1_$b.plot): COLUMN := $1
# $(foreach d,$(DS_TARGETS),$1_$d.plot): COLUMN := $1
endef
$(foreach p,$(PLOT_TYPES),$(eval $(call plot-deps,$p)))

# sets up benchmark names
define bench-deps
$1: $(foreach p,$(PLOTS),$p_$1.svg)
%_$1.plot: FILTER_NAME := $1
%_$1.plot: FILTER_DESC := $1
endef
$(foreach b,$(BENCHMARKS),$(eval $(call bench-deps,$b)))

# sets up datastructure plots
define ds-deps
$1: $(foreach p,$(PLOTS),$p_$1.svg)
%_$1.plot: FILTER_COL   := ds
%_$1.plot: FILTER_NAME  := $2
%_$1.plot: FILTER_DESC  := $2
%_$1.plot: COMPARE      := bench
endef
$(foreach d,$(DATASTRUCTURES),$(eval $(call ds-deps,$(call ds_target,$d),$d)))

# by default, scale by number of queries
FACTOR    := "queries"
PER_QUERY := "query and "
# default key position
KEYPOS    := "bottom left"
# by default, use logscale for y axis
LOGSCALE  := "set logscale y"

# preprocessing figures are not scaled by number of queries
pre%: FACTOR    := 1
pre%: PER_QUERY :=

# time measurements
time_%:           PLOT_DESC := running time
pretime_%:        PLOT_DESC := preprocessing time
time_% pretime_%: YLABEL    := " in nanoseconds"

# memory measurements
alloc_% prealloc_%:                        PLOT_DESC := memory allocation
peakmem_% prepeakmem_%:                    PLOT_DESC := peak memory usage
nalloc_% prenalloc_%:                      PLOT_DESC := number of allocations
alloc_% prealloc_% peakmem_% prepeakmem_%: YLABEL    := " in bytes"
# don't use logscale for memory
alloc_% prealloc_% peakmem_% prepeakmem_% nalloc_% prenalloc_%: LOGSCALE :=

# papi measurements
l1dcm_%:  PLOT_DESC := L1 data cache misses
l3tcm_%:  PLOT_DESC := L3 cache misses
brmsp_%:  PLOT_DESC := branch mispredictions
totins_%: PLOT_DESC := total cycles
totcyc_%: PLOT_DESC := instructions completed

COMPARE := ds
FILTER_COL := bench

# plot file
define PLOTFILE
# IMPORT-DATA results $<

# set terminal pdf size 13.33cm,10cm linewidth 2.0
set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 10 mouse jsdir '.'
set output '$(@:plot=svg)'

set pointsize 0.5
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key $(KEYPOS)
set title '$(FILTER_DESC), $(PLOT_DESC)'
set xlabel 'log₂(n)'
set ylabel '$(PLOT_DESC) per $(PER_QUERY)element (n)$(YLABEL)'

$(LOGSCALE)

## MULTIPLOT($(COMPARE)) SELECT log(2, size) as x, avg($(COLUMN))*1.0/(size*$(FACTOR)) AS y, MULTIPLOT
## FROM results WHERE $(FILTER_COL)=\"$(FILTER_NAME)\" GROUP BY MULTIPLOT,x ORDER BY MULTIPLOT,x
endef

# a newline, required for replacement below
define \n


endef

%.plot %-data.txt:
	@echo "$(subst $(\n),\n,$(PLOTFILE))" > $@
	$(SQLPLOTS) $@ 2>/dev/null

%.svg: %.plot
	gnuplot $<
