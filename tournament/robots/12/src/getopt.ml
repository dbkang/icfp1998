(* Librairie qui getopte un coup et exporte des fonctions pour faire
des choses en debug *)

(********************************)

let debug_eval=ref false;;
let debug_algo=ref false;;
let debug_alphabeta=ref false;;
let interactive=ref false;;
let max_time=ref 28;;
let max_depth=ref 100;;
let no_table = ref false;;
let mono=ref false;;

exception Unknown_argument of string;;

let rec parse=function [] -> ()
 | ("--debug-eval"::l) -> begin debug_eval:=true; parse l; end
 | ("--debug-algo"::l) -> begin debug_algo:=true; parse l; end
 | ("--debug-alphabeta"::l) -> begin debug_alphabeta:=true; parse l; end
 | ("--interactive"::l) -> begin interactive:=true; debug_algo:=true; parse l; end
 | ("--max-time"::x::l) -> begin max_time:=int_of_string x; parse l; end
 | ("--max-depth"::x::l)-> begin max_depth:=int_of_string x; parse l; end
 | ("--no-table"::l) -> begin no_table:=true; parse l; end
 | ("--mono"::l) -> begin mono:=true; parse l; end
 | (a::_) -> raise (Unknown_argument a)
;;
