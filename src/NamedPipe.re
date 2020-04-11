type t = string;

let create = (pipeName) => {
	if (Sys.win32) {
		Printf.sprintf("\\\\.\\pipe\\%s", pipeName);
	} else {
		let name =Filename.temp_file("exthost-", "-sock");
		Unix.unlink(name);
		name;
	}
}

let toString = v => v;
