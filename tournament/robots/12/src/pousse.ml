
open Misc

type state = X | O | Empty
;;

type dir = L | R | T | B
;;

let next_player =
  function X -> O | O -> X | Empty -> assert false
;;

module History = Set.Make (struct type t = int let compare = (-) end)
;;

type board =
  { size : int;
    board : state array array;
    (* Nombre de marque de chaque sorte *)
    mutable xcount : int; mutable ocount : int;
    (* Nombre de marque de chaque sorte par ligne et colonne *)
    xhsum : int array; ohsum : int array;
    xvsum : int array; ovsum : int array;
    (* Nombre de "straights" *)
    mutable xstr  : int; mutable ostr  : int;
    (* Configuration history *)
    mutable history : History.t;
    mutable history_stack : History.t list;
    (* Hachage *)
    hash_weights : int array array;
    mutable hash : int;
    (* Last player *)
    mutable player : state}
;;

(*****************************************************************)

let insert board wsum bsum n state =
  match state with
    X ->
      let s = wsum.(n) in
      if s = board.size then board.xstr <- board.xstr + 1;
      wsum.(n) <- s + 1
  | O ->
      let s = bsum.(n) in
      if s = board.size then board.ostr <- board.ostr + 1;
      bsum.(n) <- s + 1
  | Empty -> ()
;;

let remove board wsum bsum n state =
  match state with
    X ->
      let s = wsum.(n) - 1 in
      if s = board.size then board.xstr <- board.xstr - 1;
      wsum.(n) <- s
  | O ->
      let s = bsum.(n) - 1 in
      if s = board.size then board.ostr <- board.ostr - 1;
      bsum.(n) <- s
  | Empty -> ()
;;

let alpha = 65599
let extra_weight = 123787
let hash_weights size =
  let weights = Array.make_matrix (succ size) (succ size) 0 in
  let weight = ref 1 in
  for i = 0 to size do
    for j = 0 to size do
      weights.(i).(j) <- !weight;
      weight := !weight * alpha
    done
  done;
  weights
;;

let hash_insert board i j state =
  match state with
    X -> board.hash <- board.hash + board.hash_weights.(i).(j)
  | O -> board.hash <- board.hash - board.hash_weights.(i).(j)
  | Empty -> ()
;;

let hash_remove board i j state =
  match state with
    X -> board.hash <- board.hash - board.hash_weights.(i).(j)
  | O -> board.hash <- board.hash + board.hash_weights.(i).(j)
  | Empty -> ()
;;

let hash_all board =
  let sum = ref 0 in
  for i = 0 to board.size do
    for j = 0 to board.size do
      match board.board.(i).(j) with
        X -> sum := !sum + board.hash_weights.(i).(j)
      | O -> sum := !sum - board.hash_weights.(i).(j)
      | Empty -> ()
    done
  done;
  match board.player with
    X -> !sum
  | O -> !sum + extra_weight
  | Empty -> assert false
;;

let board_hash board =
  match board.player with
    X -> board.hash
  | O -> board.hash + extra_weight
  | Empty -> assert false
;;

let count_insert board state =
  match state with
    X -> board.xcount <- board.xcount + 1
  | O -> board.ocount <- board.ocount + 1
  | Empty -> ()
;;

let count_remove board state =
  match state with
    X -> board.xcount <- board.xcount - 1
  | O -> board.ocount <- board.ocount - 1
  | Empty -> ()
;;

(* Insertion d'une marque avec modification en place du tableau *)
let push board dir n mark =
  let {size = size; board = b;
       xhsum = xhsum; ohsum = ohsum;
       xvsum = xvsum; ovsum = ovsum;
       player = player} = board in
  count_insert board mark;
  board.player <- mark;
  match dir with
    L ->
      let pushed = ref b.(0).(n) in
      hash_remove board 0 n !pushed;
      remove board xvsum ovsum 0 !pushed;
      b.(0).(n) <- mark;
      hash_insert board 0 n mark;
      insert board xvsum ovsum 0 mark;
      insert board xhsum ohsum n mark;
      let i = ref 1 in
      while (!pushed <> Empty) && (!i <= size) do
        let mark = !pushed in
        pushed := b.(!i).(n);
        hash_remove board !i n !pushed;
        remove board xvsum ovsum !i !pushed;
        b.(!i).(n) <- mark;
        hash_insert board !i n mark;
        insert board xvsum ovsum !i mark;
        incr i
      done;
      remove board xhsum ohsum n !pushed;
      count_remove board !pushed;
      (!i, !pushed)
  | R ->
      let pushed = ref b.(size).(n) in
      hash_remove board size n !pushed;
      remove board xvsum ovsum size !pushed;
      b.(size).(n) <- mark;
      hash_insert board size n mark;
      insert board xvsum ovsum size mark;
      insert board xhsum ohsum n mark;
      let i = ref (size - 1) in
      while (!pushed <> Empty) && (!i >= 0) do
        let mark = !pushed in
        pushed := b.(!i).(n);
        hash_remove board !i n !pushed;
        remove board xvsum ovsum !i !pushed;
        b.(!i).(n) <- mark;
        hash_insert board !i n mark;
        insert board xvsum ovsum !i mark;
        decr i
      done;
      remove board xhsum ohsum n !pushed;
      count_remove board !pushed;
      (!i, !pushed)
  | T ->
      let pushed = ref b.(n).(0) in
      hash_remove board n 0 !pushed;
      remove board xhsum ohsum 0 !pushed;
      b.(n).(0) <- mark;
      hash_insert board n 0 mark;
      insert board xhsum ohsum 0 mark;
      insert board xvsum ovsum n mark;
      let i = ref 1 in
      while (!pushed <> Empty) && (!i <= size) do
        let mark = !pushed in
        pushed := b.(n).(!i);
        hash_remove board n !i !pushed;
        remove board xhsum ohsum !i !pushed;
        b.(n).(!i) <- mark;
        hash_insert board n !i mark;
        insert board xhsum ohsum !i mark;
        incr i
      done;
      remove board xvsum ovsum n !pushed;
      count_remove board !pushed;
      (!i, !pushed)
  | B ->
      let pushed = ref b.(n).(size) in
      hash_remove board n size !pushed;
      remove board xhsum ohsum size !pushed;
      b.(n).(size) <- mark;
      hash_insert board n size mark;
      insert board xhsum ohsum size mark;
      insert board xvsum ovsum n mark;
      let i = ref (size - 1) in
      while (!pushed <> Empty) && (!i >= 0) do
        let mark = !pushed in
        pushed := b.(n).(!i);
        hash_remove board n !i !pushed;
        remove board xhsum ohsum !i !pushed;
        b.(n).(!i) <- mark;
        hash_insert board n !i mark;
        insert board xhsum ohsum !i mark;
        decr i
      done;
      remove board xvsum ovsum n !pushed;
      count_remove board !pushed;
      (!i, !pushed)
;;

(* Annule un coup *)
let unpush board dir n (last, mark) =
  let {size = size; board = b;
       xhsum = xhsum; ohsum = ohsum;
       xvsum = xvsum; ovsum = ovsum} = board in
  count_insert board mark;
  match dir with
    L ->
      let pushed = b.(0).(n) in
      board.player <- next_player pushed;
      count_remove board pushed;
      hash_remove board 0 n pushed;
      remove board xvsum ovsum 0 pushed;
      remove board xhsum ohsum n pushed;
      for i = 0 to last - 2 do
        let pushed = b.(i + 1).(n) in
        hash_remove board (i + 1) n pushed;
        remove board xvsum ovsum (i + 1) pushed;
        b.(i).(n) <- pushed;
        hash_insert board i n pushed;
        insert board xvsum ovsum i pushed
      done;
      b.(last - 1).(n) <- mark;
      hash_insert board (last - 1) n mark;
      insert board xvsum ovsum (last - 1) mark;
      insert board xhsum ohsum n mark
  | R ->
      let pushed = b.(size).(n) in
      board.player <- next_player pushed;
      count_remove board pushed;
      hash_remove board size n pushed;
      remove board xvsum ovsum size pushed;
      remove board xhsum ohsum n pushed;
      for i = size downto last + 2 do
        let pushed = b.(i - 1).(n) in
        hash_remove board (i - 1) n pushed;
        remove board xvsum ovsum (i - 1) pushed;
        b.(i).(n) <- pushed;
        hash_insert board i n pushed;
        insert board xvsum ovsum i pushed
      done;
      b.(last + 1).(n) <- mark;
      hash_insert board (last + 1) n mark;
      insert board xvsum ovsum (last + 1) mark;
      insert board xhsum ohsum n mark
  | T ->
      let pushed = b.(n).(0) in
      board.player <- next_player pushed;
      count_remove board pushed;
      hash_remove board n 0 pushed;
      remove board xhsum ohsum 0 pushed;
      remove board xvsum ovsum n pushed;
      for i = 0 to last - 2 do
        let pushed = b.(n).(i + 1) in
        hash_remove board n (i + 1) pushed;
        remove board xhsum ohsum (i + 1) pushed;
        b.(n).(i) <- pushed;
        hash_insert board n i pushed;
        insert board xhsum ohsum i pushed
      done;
      b.(n).(last - 1) <- mark;
      hash_insert board n (last - 1) mark;
      insert board xhsum ohsum (last - 1) mark;
      insert board xvsum ovsum n mark
  | B ->
      let pushed = b.(n).(size) in
      board.player <- next_player pushed;
      count_remove board pushed;
      hash_remove board n size pushed;
      remove board xhsum ohsum size pushed;
      remove board xvsum ovsum n pushed;
      for i = size downto last + 2 do
        let pushed = b.(n).(i - 1) in
        hash_remove board n (i - 1) pushed;
        remove board xhsum ohsum (i - 1) pushed;
        b.(n).(i) <- pushed;
        hash_insert board n i pushed;
        insert board xhsum ohsum i pushed
      done;
      b.(n).(last + 1) <- mark;
      hash_insert board n (last + 1) mark;
      insert board xhsum ohsum (last + 1) mark;
      insert board xvsum ovsum n mark
;;

let play board (dir, n, player) = push board dir n player
let unplay board (dir, n, _) undo = unpush board dir n undo

let add_to_history board =
  board.history_stack <- board.history :: board.history_stack;
  board.history <- History.add (board_hash board) board.history 
;;

let remove_from_history board =
  match board.history_stack with
    history :: stack ->
      board.history_stack <- stack;
      board.history <- history
  | [] ->
      assert false
;;

(*****************************************************************)

let win = max_int
let loose = - max_int

let is_leaf board =
  History.mem (board_hash board) board.history
    ||
  board.xstr <> board.ostr
;;

(*****************************************************************)

(* Tableau vide *)
let empty size =
  { size = size;
    board = Array.make_matrix (succ size) (succ size) Empty;
    xcount = 0; ocount = 0;
    xhsum = Array.make (succ size) 0;
    ohsum = Array.make (succ size) 0;
    xvsum = Array.make (succ size) 0;
    ovsum = Array.make (succ size) 0;
    ostr = 0; xstr = 0;
    history = History.empty;
    history_stack = [];
    hash_weights = hash_weights size;
    hash = 0;
    player = O}
;;

(* Duplication d'un tableau *)
let copy board =
  { size = board.size;
    board = Array.map Array.copy board.board;
    xcount = board.xcount; ocount = board.ocount;
    xhsum = Array.copy board.xhsum;
    ohsum = Array.copy board.ohsum;
    xvsum = Array.copy board.xvsum;
    ovsum = Array.copy board.ovsum;
    ostr = board.ostr; xstr = board.xstr;
    history = board.history;
    history_stack = board.history_stack;
    hash_weights = board.hash_weights;
    hash = board.hash;
    player = board.player}
;;

(*****************************************************************)

let read_move ch =
  let dir =
    match input_char ch with
      'L' -> L | 'R' -> R | 'T' -> T | 'B' -> B
    | _ -> assert false
  in
  let n = int_of_string (input_line ch) - 1 in
  (dir, n)

let print_move outch dir n =
  output_char outch
    (match dir with
       L -> 'L' | R -> 'R' | T -> 'T' | B -> 'B');
  output_int outch (n + 1)

(*****************************************************************)

let smove = ref (-1)
let xmoves = ref []
let omoves = ref []
let possible_moves board =
  if !smove <> board.size then begin
    smove := board.size;
    let s = board.size + 1 in
    let xm = Array.make (4 * s) (L, 0, X) in
    let om = Array.make (4 * s) (L, 0, O) in
    for i = 0 to board.size do
      xm.(i)         <- (L, i, X);
      xm.(i + s)     <- (R, i, X);
      xm.(i + 2 * s) <- (T, i, X);
      xm.(i + 3 * s) <- (B, i, X);
      om.(i)         <- (L, i, O);
      om.(i + s)     <- (R, i, O);
      om.(i + 2 * s) <- (T, i, O);
      om.(i + 3 * s) <- (B, i, O)
    done;
    xmoves := Array.to_list xm;
    omoves := Array.to_list om
  end;
  match board.player with
    X -> !omoves | O -> !xmoves | Empty -> assert false
