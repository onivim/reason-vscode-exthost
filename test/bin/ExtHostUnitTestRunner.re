prerr_endline(Sys.getcwd());
Timber.App.enable();
Timber.App.setLevel(Timber.Level.trace);
ExtHostTest.TestFramework.cli();
ExtHostTransportTest.TestFramework.cli();
ExtHostExtensionTest.TestFramework.cli();
ExtHostWhenExprTests.TestFramework.cli();
ExtHostTypesTests.TestFramework.cli();
