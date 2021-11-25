# LC
#[=[
ML
C
]=]
#[===[BC]===] #[[ABC]]#LC

no_arguments()
 space_first_no_arguments()
space_after_no_arguments ()

bracket_argument([===[arg1
${VAR}]===])
quoted_argument("arg1 ${VAR}")
unquoted_argument(arg1${VAR})
unquoted_legacy_argument(arg="1")
unquoted_legacy_argument2(arg=$(1))

space_before_argument( arg1)
space_after_argument(arg1 )

multi_argument([=====[arg1]=====] "arg2" arg3)

multiline_command(
arg1
arg2
)

arguments_in_arguments(
	([====[arg1]====] "arg2" arg3)
)

escape_characters("\r\t\n\;" \)a)
