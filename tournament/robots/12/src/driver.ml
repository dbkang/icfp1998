open Misc
open Pousse
open Timeout

let size = int_of_string Sys.argv.(1)
let prg1 = Sys.argv.(2)
let prg2 = Sys.argv.(3)

let counter = ref 1
let board = empty (size - 1)
let moves = ref []

let _ =
while true do
  Printer.board board;
  if abs (Alphabeta.eval board) = max_int then begin
    let prg = match board.player with X -> prg1 | _ -> prg2 in
    if Alphabeta.eval board = max_int then
      Printf.printf "Player %s has won" prg
    else
      Printf.printf "Player %s has lost" prg;
    print_newline ();
    exit 0
  end;
  let player = next_player board.player in
  let prg = match player with X -> prg1 | _ -> prg2 in
  Printf.eprintf "Move %d - player %s (%c):" !counter prg
    (match player with X -> 'X' | _ -> 'O');
  prerr_newline ();
  let (dir, n) as move =
    try
      global_time_out 30 (fun _ ->
        let (inch, outch) = Unix.open_process prg in
        output_int outch size;
        output_char outch '\n';
        List.iter
          (fun (dir, n) ->
            print_move outch dir n;
            output_char outch '\n')
          (List.rev !moves);
        flush outch;
        close_out outch;
        let move = read_move inch in
        Unix.close_process (inch, outch);
        move)
    with Timed_out ->
      Printf.printf "Player %s has timed out" prg;
      print_newline ();
      exit 0
  in
  incr counter;
  moves := move :: !moves;
  add_to_history board;
  push board dir n player
done
