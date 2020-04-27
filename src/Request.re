module Commands = {
  let executeContributedCommand = (~arguments, ~command, client) => {
    Client.notify(
	~rpcName="ExtHostCommands",
	~method="$executeContributedCommand",
	~args=`List([`String(command), ...arguments]),
	client
	);
  }
};
