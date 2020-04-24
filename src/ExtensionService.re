[@deriving show]
type msg =
  | ActivateExtension({
      extensionId: string,
      activationEvent: option(string),
    })
  | OnWillActivateExtension({extensionId: string})
  | OnDidActivateExtension({
      extensionId: string,
      //startup: bool,
      codeLoadingTime: int,
      activateCallTime: int,
      activateResolvedTime: int,
    })
  //activationEvent: option(string),
  | OnExtensionActivationFailed({extensionId: string})
  | OnExtensionRuntimeError({extensionId: string});
// TODO: Error?

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$activateExtension", `List([`String(extensionId)])) =>
    Ok(ActivateExtension({extensionId, activationEvent: None}))
  | (
      "$activateExtension",
      `List([`String(extensionId), activationEventJson]),
    ) =>
    let activationEvent =
      switch (activationEventJson) {
      | `String(v) => Some(v)
      | _ => None
      };

    Ok(ActivateExtension({extensionId, activationEvent}));
  | ("$onWillActivateExtension", `List([`String(extensionId)])) =>
    Ok(OnWillActivateExtension({extensionId: extensionId}))
  | (
      "$onDidActivateExtension",
      `List([
        `String(extensionId),
        `Int(codeLoadingTime),
        `Int(activateCallTime),
        `Int(activateResolvedTime),
        ..._args,
      ]),
    ) =>
    Ok(
      OnDidActivateExtension({
        extensionId,
        codeLoadingTime,
        activateCallTime,
        activateResolvedTime,
      }),
    )
  | ("$onExtensionActivatedFailed", `List([`String(extensionId), ..._args])) =>
    Ok(OnExtensionActivationFailed({extensionId: extensionId}))
  | ("$onExtensionRuntimeError", `List([`String(extensionId), ..._args])) =>
    Ok(OnExtensionRuntimeError({extensionId: extensionId}))
  | _ => Error("Unhandled method: " ++ method)
  };
};
