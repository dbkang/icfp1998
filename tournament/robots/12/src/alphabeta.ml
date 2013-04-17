
open Pousse

let hash_table = Array.make 10007 (-1, (0, (-1, Some (L, 0, X))))

let hash_find h =
  let (h', v) = hash_table.((h land max_int) mod 997) in
  if h = h' then Some v
  else None

let hash_add h d v =
  let h0 = (h land max_int) mod 997 in
  let (_, (d', _)) = hash_table.(h0) in
  if d' <= d then
    hash_table.(h0) <- (h, (d, v))

let rec list_except l v =
  match l with
    []                -> []
  | a :: l when a = v -> l
  | a :: l            -> a :: list_except l v

(**************************************************************)

let eval board =
  if History.mem (board_hash board) board.history then
    loose
  else if board.xstr = board.ostr then
    Eval.f board
  else
    let score = if board.xstr > board.ostr then win else loose in
    match board.player with X -> score | O -> - score | _ -> assert false

(**************************************************************)

let rec fold depth board alpha beta best best_move =
  function
    [] ->
      (-best, best_move)
  | move::rem ->
      let best = ref best in
      let best_move = ref best_move in

      let alpha = if alpha < !best then !best else alpha in
      let undo = play board move in
      let (v, _) = f depth board (-beta) (-alpha) in
      if !best < v then begin
        best := v;
        best_move := Some move
      end;
      unplay board move undo;
      if !best >= beta then (- !best, !best_move) else
        fold depth board alpha beta !best !best_move rem

and f depth board alpha beta =
  if (depth = 0) || (is_leaf board) then
    (eval board, None)
  else begin
    add_to_history board;
    let res =
      if not (!Getopt.no_table) then
        let hash = board_hash board in
        (* Try to first consider a good move *)
        let res =
          match hash_find hash with
            Some (d, (_, Some move)) ->
              fold (pred depth) board alpha beta loose None
                (move::list_except (possible_moves board) move)
          | _ ->
              fold (pred depth) board alpha beta loose None
                (possible_moves board)
        in
          hash_add hash depth res;
          res
      else
        fold (pred depth) board alpha beta loose None
          (possible_moves board)
    in
    remove_from_history board;
    res
  end

(**************************************************************)

(*
  4 processus crees initialement.
  Le process principal leur envoie chacun un coup.
  Chaque fois qu'ils ont fini, ils renvoient l'evaluation de leur
  coup et recoivent un nouveau coup (de preference de leur cote)
  On les tue au timeout

Table des process :
  desc entree -> infos sur le process
*)

let create_process child =
  let (in_read, in_write) = Unix.pipe() in
  let (out_read, out_write) = Unix.pipe() in
  let inchan = Unix.in_channel_of_descr in_read in
  let outchan = Unix.out_channel_of_descr out_write in
  match Unix.fork() with
     0 -> Unix.dup2 out_read Unix.stdin;
          Unix.dup2 in_write Unix.stdout;
          Unix.close out_read; Unix.close in_read;
          Unix.close out_write; Unix.close in_write;
          child ();
          exit 127
  | id -> Unix.close out_read; Unix.close in_write;
          (id, inchan, outchan)

type process = {
  pid : int;
  inch : in_channel;
  outch : out_channel;
  dir : dir;
  mutable move : dir * int * state
}
let processes = Hashtbl.create 7
let processes_output = ref []

let kill_all () =
  Hashtbl.iter
    (fun _ p ->
       Unix.kill p.pid Sys.sigterm; close_in p.inch; close_out p.outch)
    processes;
  Hashtbl.clear processes;
  processes_output := []

let create_processes f =
  List.map
    (fun dir ->
       let (pid, inch, outch) = create_process f in
       let p =
         { pid = pid;
           inch = inch;
           outch = outch;
           dir = dir;
           move =  (T, 0, X) (* dummy move *) }
       in
       let inch = Unix.descr_of_in_channel inch in
       Hashtbl.add processes inch p;
       processes_output := inch :: !processes_output;
       p)
    [L; R; T; B]

let rec child board () =
  let (move, depth, alpha) = input_value stdin in
  let undo = play board move in
  let (v, _) = f (pred depth) board loose (- alpha) in
  output_value stdout v;
  flush stdout;
  unplay board move undo;
  child board ()

let start_move p move depth alpha =
  output_value p.outch (move, depth, alpha);
  flush p.outch

let read_move p =
  input_value p.inch

let wait_for_child () =
  let (pr, _, _) = Unix.select !processes_output [] [] (-1.) in
  List.map (Hashtbl.find processes) pr

let rec find_good_move dir =
  function
    []   ->
      raise Exit
  | (dir', _, _) as m::l when dir = dir' ->
      (m, l)
  | m::l ->
      let (m', l') = find_good_move dir l in
      (m', m::l')

let find_move dir moves =
  try
    Some (find_good_move dir moves)
  with Exit ->
    match moves with
      m::l -> Some (m, l)
    | []   -> None
    
let para_find max_depth board =
  let best_move = ref (T, 0, X) in

  let initial_moves = ref (possible_moves board) in
  let best_initial_move = ref (T, -1, X) in
  let consider_new_moves = ref false in
  let moves = ref [] in
  let order_moves moves =
    Sort.list
     (fun ((v, i), _) ((v', i'), _) -> (v > v') || ((v = v') && (i <= i')))
     moves
  in
  begin try
    add_to_history board;
    let avail = ref (create_processes (child board)) in
    let depth = ref 1 in
    if !Getopt.debug_algo then Printf.eprintf "Depth:"; flush stderr;
    while !depth <= max_depth do
      if !Getopt.debug_algo then begin
        Printf.eprintf " %d" !depth; flush stderr
      end;
      let alpha = ref loose in
      let index = ref 0 in

      let rem_moves = ref !initial_moves in
      best_initial_move := List.hd !initial_moves;
      consider_new_moves := false;
      while
        let pr = !avail in
        avail := [];
        List.iter
          (fun p ->
             match find_move p.dir !rem_moves with
               None      ->
                 avail := p :: !avail
             | Some (move, rem) ->
                 rem_moves := rem;
                 p.move <- move;
                 start_move p move !depth !alpha)
          pr;
        List.length !avail <> 4
      do
        let finished = wait_for_child () in
        List.iter
          (fun p ->
             let v = read_move p in
             if !Getopt.debug_alphabeta then begin
               Printf.eprintf " [%d]" v;
               flush stderr
             end else if !Getopt.debug_algo then begin
               if v = win then prerr_string "!"
               else if v = loose then prerr_string "?"
               else prerr_string ".";
               if p.move = !best_initial_move then
                 prerr_string "*";
               flush stderr;
             end;
             if v <> loose then
               moves := ((v, !index), p.move):: !moves;
             if !alpha < v then alpha := v;
             incr index;
             if p.move = !best_initial_move || v = win then
               consider_new_moves := true;
             if v = win then raise Exit)
          finished;
        avail := finished @ !avail
      done;
      let ml = order_moves !moves in
      begin match ml with
        [_] | [] ->
          raise Exit
      | ((sc, _), move)::_ ->
          if !Getopt.debug_algo then begin
            let (dir, n, _) = move in print_move stderr dir n; flush stderr;
            Printf.eprintf " (%d) " sc
          end;
      end;
      initial_moves := List.map snd ml;
      moves := [];
      incr depth
    done
  with Exit | Timeout.Timed_out -> () end;
  kill_all ();
  match order_moves !moves with
    ((sc, _), move)::_ when !consider_new_moves ->
      if !Getopt.debug_algo then begin
        let (dir, n, _) = move in print_move stderr dir n; flush stderr;
        Printf.eprintf " (%d) " sc;
        prerr_newline ()
      end;
      move
  | _ ->
      if !Getopt.debug_algo then begin
        let (dir, n, _) = !best_initial_move in
        print_move stderr dir n; flush stderr;
        prerr_newline ()
      end;
      !best_initial_move

(**************************************************************)

let mono_find max_depth board =
  let best_move = ref (T, 0, X) in

  let initial_moves = ref (possible_moves board) in
  let moves = ref [] in
  let order_moves moves =
    Sort.list
     (fun ((v, i), _) ((v', i'), _) -> (v > v') || ((v = v') && (i <= i')))
     moves
  in
  begin try
    let depth = ref 1 in
    if !Getopt.debug_algo then Printf.eprintf "Depth:"; flush stderr;
    while !depth <= max_depth do
      if !Getopt.debug_algo then begin
        Printf.eprintf " %d" !depth; flush stderr
      end;
      let alpha = ref loose in
      add_to_history board;
      let index = ref 0 in
      List.iter
        (fun move ->
           let undo = play board move in
           let (v, _) = f (pred !depth) board loose (- !alpha) in
           if !Getopt.debug_alphabeta then begin
             Printf.eprintf " [%d]" v;
             flush stderr
           end else if !Getopt.debug_algo then begin
             if v = win then prerr_string "!"
             else if v = loose then prerr_string "?"
             else prerr_string ".";
             flush stderr;
           end;
           if v <> loose then
             moves := ((v, !index), move):: !moves;
           if !alpha < v then alpha := v;
           unplay board move undo;
           incr index;
           if v = win then raise Exit)
        !initial_moves;
      let ml = order_moves !moves in
      begin match ml with
        [_] | [] ->
          raise Exit
      | ((sc, _), move)::_ ->
          if !Getopt.debug_algo then begin
            let (dir, n, _) = move in print_move stderr dir n; flush stderr;
            Printf.eprintf " (%d) " sc
          end;
      end;
      initial_moves := List.map snd ml;
      moves := [];
      incr depth
    done
  with Exit | Timeout.Timed_out -> () end;
  match order_moves !moves with
    ((sc, _), move)::_ ->
      if !Getopt.debug_algo then begin
        let (dir, n, _) = move in print_move stderr dir n; flush stderr;
        Printf.eprintf " (%d) " sc;
        prerr_newline ()
      end;
      move
  | _ ->
      let move = List.hd !initial_moves in
      if !Getopt.debug_algo then begin
        let (dir, n, _) = move in print_move stderr dir n; flush stderr;
        prerr_newline ()
      end;
      move

let find max_depth board =
  if !Getopt.mono then mono_find max_depth board
                  else para_find max_depth board
