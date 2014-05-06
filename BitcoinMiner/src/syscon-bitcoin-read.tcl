set bitcoin_miner 0x0

set m [lindex [get_service_paths master] 0]
open_service master $m
puts "Opened master"

set hb ""
for {set i 95} {$i >= 0} {incr i -1} {
	set tmp [master_read_8 $m [expr $bitcoin_miner + $i] 1]
	append hb " $tmp"
}

set nonce ""
for {set i 99} {$i > 95} {incr i -1} {
	set tmp [master_read_8 $m [expr $bitcoin_miner + $i] 1]
	append nonce " $tmp"
}

#set n_out ""
#for {set i 107} {$i > 103} {incr i -1} {
#	set tmp [master_read_8 $m [expr $bitcoin_miner + $i] 1]
#	append n_out " $tmp"
#}

set start [master_read_8 $m [expr $bitcoin_miner + 102] 1]
set loading [master_read_8 $m [expr $bitcoin_miner + 101] 1]
set ticket [master_read_8 $m [expr $bitcoin_miner + 100] 1]
set nstate [master_read_8 $m [expr $bitcoin_miner + 103] 1]

puts "Output:"
puts "header_buffer:\n$hb"
puts "<stop><start>: $start"
puts "load state: $loading"
puts "nonce: $nonce"
#puts "n_out: $n_out"
puts "ticket: $ticket"
puts "nstate <nonce_out32><read_gold_nonce>: $nstate" 

close_service master $m
puts "Closed master"
