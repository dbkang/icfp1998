
open Pousse

let sqr x = x * x
let eval_side size count hsum vsum =
  let s = size + 1 in
  let sum = ref 0 in
  for i = 0 to size do
    sum := !sum + sqr (s * hsum.(i) - count)
                + sqr (s * vsum.(i) - count)
  done;
  !sum + count * count * count * 1000
;;
(*
let eval_side size count hsum vsum =
  let s = size + 1 in
  let sum = ref 0 in
  for i = 0 to size do
    sum := !sum + abs (s * hsum.(i) - count)
                + abs (s * vsum.(i) - count)
  done;
  !sum + count
;;
*)
let f board =
  let score =
    (eval_side board.size board.xcount board.xhsum board.xvsum
       -
     eval_side board.size board.ocount board.ohsum board.ovsum)
  in
  match board.player with X -> score | O -> - score | _ -> assert false
