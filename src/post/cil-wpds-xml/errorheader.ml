open Cil
open Printf
open List
module P = Pretty

  
type funcNode = {
    dirname: string;
    filename: string;
    fcname: string;
    floc: Cil.location;
    fd: Cil.fundec;     
    mutable callers: funcEdge list;
    mutable callees: funcEdge list;

    mutable nRetvalEIO: errornode;
    mutable nArgEIO: errornode;
    mutable nSubmit: submitnode;

    mutable visited: svisit;
    mutable nColor: string;

    mutable isVisited: bool; (*cindy*)
  }

and funcEdge = {
    mutable caller: funcNode;
    mutable callee: funcNode;
    mutable eInstr: instr;
    mutable eLoc: location;
    mutable eRetvalEIO: erroredge;
    mutable eArgEIO: erroredge;
    mutable eColor: string;
    mutable eClass1: string; (* classification *)
    mutable eClass2: string;
    mutable eClass3: string;

    mutable visitado: bool; (*cindy*)
  }      

and svisit = {
    mutable any: bool;
    mutable up: bool;
    mutable down: bool
  }

and submitnode = {
    mutable submit: bool;
    mutable wait: bool;
    mutable check: bool;
  }
      
and errornode = {
    mutable direct: bool;
    mutable propagate: bool;
    mutable involve: bool;
    mutable errloc: int;
  }

and erroredge = {
    mutable edgePropagate: bool;
    mutable callerPropagate: bool;
    mutable endpoint: bool;
    mutable endpointSaved: bool;
    mutable endpointChecked: bool;
    mutable endpointVarInfo: varinfo;
    mutable endpointChangeMechanism: bool;  
    (* e.g. from retval to function pointer or vice versa *)
  }
      
type callGraph = 
    (string, funcNode) Hashtbl.t 


and boolarg = {
    mutable bval: bool;
  }      
      
and intarg = {
    mutable ival: int;
  }      




