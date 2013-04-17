
open Pousse

let board b =
  let s = b.size in
  for i = 0 to s do
    for j = 0 to s do
      prerr_string "+-";
    done;
    prerr_string "+";
    prerr_newline ();
    for j = 0 to s do
      prerr_string "|";
      prerr_string 
        (match b.board.(j).(i) with
           X -> "X"
         | O -> "O"
         | Empty -> " ")
    done;
    prerr_string "| ";
    Printf.eprintf "%2d %2d" b.xhsum.(i) b.ohsum.(i);
    prerr_newline ()
  done;
  for j = 0 to s do
    prerr_string "+-";
  done;
  prerr_string "+";
  prerr_newline ();
  for j = 0 to s do
    Printf.eprintf "%2d" b.xvsum.(j)
  done;
  prerr_string "   X    ";
  Printf.eprintf "%3d marks / %2d straights" b.xcount b.xstr;
  prerr_newline ();
  for j = 0 to s do
    Printf.eprintf "%2d" b.ovsum.(j)
  done;
  prerr_string "      O ";
  Printf.eprintf "%3d marks / %2d straights" b.ocount b.ostr;
  prerr_newline ();
  prerr_newline ()
;;
