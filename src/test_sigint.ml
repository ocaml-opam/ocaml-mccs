external loop : unit -> unit = "mccs_test_sigint"

let () = Callback.register_exception "Sys.Break" Sys.Break

let () =
  while true
  do
    try
      loop ()
    with Sys.Break ->
      Printf.eprintf "User pressed CTRL+C!\n%!"
  done
