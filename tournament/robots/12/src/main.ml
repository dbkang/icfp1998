
(*
  - Optimiser Pousse.push et Pousse.unpush (?)
*)

open Pousse

let think_about_it=fun board->
  let board=Pousse.copy board in
  if !Getopt.interactive then begin
        Printer.board board;
        prerr_string "Player: ";
        prerr_string
          (match next_player board.player with
             X -> "X"
           | O -> "O"
           | Empty -> " ");
        prerr_newline ();
  end;
  let best_move=
    if !Getopt.interactive then
      Timeout.global_time_out !Getopt.max_time (fun () ->
        Alphabeta.find !Getopt.max_depth board)
    else
      Alphabeta.find !Getopt.max_depth board
  in
  if !Getopt.debug_algo then prerr_newline ();
  if !Getopt.interactive then begin 
     Printf.eprintf "Suggested move : ";
     prerr_newline();
  end;
  begin match best_move with
    dir, n, sc -> print_move stdout dir n; print_newline ();
  end;;

let init () =
  if !Getopt.interactive then begin 
    Printf.eprintf "Size of board :";
    prerr_newline ();
  end;
  let size = int_of_string (read_line ()) in
  let board = empty (size - 1) in
  begin try
    while true do
      if !Getopt.interactive then begin
	    think_about_it board;
      end;
      let (dir, n) = read_move stdin in
      add_to_history board;
      push board dir n (next_player board.player);
      if !Getopt.debug_eval then begin
        Printer.board board;
        Printf.eprintf "Current score : %d" (Alphabeta.eval board);
        prerr_newline ()
      end
    done
  with End_of_file -> () end;
  board

let _ =
  let _ = Getopt.parse (List.tl (Array.to_list Sys.argv)) in
  if !Getopt.interactive then
    let board = init () in think_about_it board
  else
    Timeout.global_time_out !Getopt.max_time (fun () ->
      let board = init () in think_about_it board);;
