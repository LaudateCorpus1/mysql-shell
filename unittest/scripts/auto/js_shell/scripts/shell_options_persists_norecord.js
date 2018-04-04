var _defaultSandboxDir = shell.options.sandboxDir

//@ shell classic connection
shell.connect(__mysqluripwd)

//@ autocomplete.nameCache update and set back to default using shell.options
shell.options.set_persist("autocomplete.nameCache", false);
shell.options["autocomplete.nameCache"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("autocomplete.nameCache")
shell.options["autocomplete.nameCache"]

//@ batchContinueOnError update and set back to default using shell.options
shell.options.set("batchContinueOnError", true);
shell.options["batchContinueOnError"]
shell.options.unset_persist("batchContinueOnError")
shell.options["batchContinueOnError"]

//@ devapi.dbObjectHandles update and set back to default using shell.options
shell.options.set_persist("devapi.dbObjectHandles", false);
shell.options["devapi.dbObjectHandles"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("devapi.dbObjectHandles")
shell.options["devapi.dbObjectHandles"]

//@ history.autoSave update and set back to default using shell.options
shell.options.set_persist("history.autoSave", true);
shell.options["history.autoSave"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("history.autoSave");
shell.options["history.autoSave"]

//@ history.maxSize update and set back to default using shell.options
shell.options.set_persist("history.maxSize", 10);
shell.options["history.maxSize"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("history.maxSize");
shell.options["history.maxSize"]

//@ history.sql.ignorePattern update and set back to default using shell.options
shell.options.set_persist("history.sql.ignorePattern", "*PATTERN*");
shell.options["history.sql.ignorePattern"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("history.sql.ignorePattern");
shell.options["history.sql.ignorePattern"]

//@ interactive update and set back to default using shell.options
shell.options["interactive"] = "true"
shell.options["interactive"]
shell.options.unset_persist("interactive");
shell.options["interactive"]

//@ logLevel update and set back to default using shell.options
shell.options.set_persist("logLevel", 8);
shell.options["logLevel"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("logLevel");
shell.options["logLevel"]

//@ outputFormat update and set back to default using shell.options
shell.options.set_persist("outputFormat", "json/raw");
shell.options["outputFormat"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("outputFormat")
shell.options["outputFormat"]

//@ passwordsFromStdin update and set back to default using shell.options
shell.options.set_persist("passwordsFromStdin", true);
shell.options["passwordsFromStdin"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("passwordsFromStdin")
shell.options["passwordsFromStdin"]

//@ sandboxDir update and set back to default using shell.options
shell.options.set_persist("sandboxDir", "\\sandboxDir");
shell.options["sandboxDir"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("sandboxDir");
shell.options["sandboxDir"]

//@ showWarnings update and set back to default using shell.options
shell.options.set_persist("showWarnings", false);
shell.options["showWarnings"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("showWarnings");
shell.options["showWarnings"]

//@ useWizards update and set back to default using shell.options
shell.options.set_persist("useWizards", false);
shell.options["useWizards"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("useWizards");
shell.options["useWizards"]

//@ defaultMode update and set back to default using shell.options
shell.options.set_persist("defaultMode", "py");
shell.options["defaultMode"]
os.load_text_file(os.get_user_config_path() + "/test_options.json")
shell.options.unset_persist("defaultMode");
shell.options["defaultMode"]

//@ autocomplete.nameCache update and set back to default using \option
\option --persist autocomplete.nameCache = false
\option autocomplete.nameCache
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist autocomplete.nameCache
\option autocomplete.nameCache

//@ batchContinueOnError update and set back to default using \option
\option batchContinueOnError = true
\option batchContinueOnError
\option --unset --persist batchContinueOnError
\option batchContinueOnError

//@ devapi.dbObjectHandles update and set back to default using \option
\option --persist devapi.dbObjectHandles = false
\option devapi.dbObjectHandles
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist devapi.dbObjectHandles
\option devapi.dbObjectHandles

//@ history.autoSave update and set back to default using \option
\option --persist history.autoSave = true
\option history.autoSave
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist history.autoSave
\option history.autoSave

//@ history.maxSize update and set back to default using \option
\option --persist history.maxSize = 10
\option history.maxSize
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist history.maxSize
\option history.maxSize

//@ history.sql.ignorePattern update and set back to default using \option
\option --persist history.sql.ignorePattern = *PATTERN*
\option history.sql.ignorePattern
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist history.sql.ignorePattern
\option history.sql.ignorePattern

//@ interactive update and set back to default using \option
\option interactive = true
\option interactive
\option --unset --persist interactive
\option interactive

//@ logLevel update and set back to default using \option
\option --persist logLevel = 8
\option logLevel
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist logLevel
\option logLevel

//@ outputFormat update and set back to default using \option
\option --persist outputFormat = json/raw
\option outputFormat
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist outputFormat
\option outputFormat

//@ passwordsFromStdin update and set back to default using \option
\option --persist passwordsFromStdin = true
\option passwordsFromStdin
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist passwordsFromStdin
\option passwordsFromStdin

//@ sandboxDir update and set back to default using \option
\option --persist sandboxDir = \sandboxDir
\option sandboxDir
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist sandboxDir
\option sandboxDir

//@ showWarnings update and set back to default using \option
\option --persist showWarnings = false
\option showWarnings
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist showWarnings
\option showWarnings

//@ useWizards update and set back to default using \option
\option --persist useWizards = false
\option useWizards
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist useWizards
\option useWizards

//@ defaultMode update and set back to default using \option
\option --persist defaultMode = py
\option defaultMode
os.load_text_file(os.get_user_config_path() + "/test_options.json")
\option --unset --persist defaultMode
\option defaultMode

//@ List all the options using \option
\option -l

//@ List all the options using \option and show-origin
\option --list --show-origin

//@ List an option which origin is Compiled default
\option --list --show-origin

//@ List an option which origin is User defined
\option logLevel=8
\option -l --show-origin

//@ Verify error messages
shell.options.defaultMode = 1
\option defaultMode = 1
shell.options.unset(InvalidOption)
\option --unset InvalidOption
\option -h InvalidOption
\option --help InvalidOption

//@ Configuration operation available in SQL mode
\sql
\option logLevel
\option logLevel=0
\option logLevel
\option --unset logLevel
\option logLevel

//@ List all the options using \option for SQL mode
\option --list
\js

//@ List all the options using \option and show-origin for SQL mode
\sql
\option --list --show-origin
\js