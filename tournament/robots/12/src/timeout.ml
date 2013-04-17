exception Timed_out

let install_signal_handler signal behav =
  Sys.signal signal behav
  ; Sys.Signal_default (* needed for OCaml 1.07 compatibility *)
  
let global_time_out time_out_delay try_me =
  let savedsig = ref Sys.Signal_default in
  let restore_sig () = Sys.signal Sys.sigalrm !savedsig in
  let do_time_out = fun _ ->
	begin
	  restore_sig () ;
	  raise Timed_out
	end in
  savedsig := install_signal_handler
		Sys.sigalrm (Sys.Signal_handle do_time_out) ;
  Unix.alarm time_out_delay ;
  let result = try_me () in
  restore_sig () ;
  result
