function(no_arguments)
	printVars()
endfunction()
no_arguments()

function(one_argument var)
	printVars()
endfunction()
one_argument("Hi")

function(two_arguments var1, var2)
	printVars()
endfunction()
two_arguments("Hi" "Again")

printVars()
set(var "Hello")
printVars()

function(functionScope)
	set(anotherVar "Nice")
	printVars()
endfunction()
functionScope()
printVars()

function(macroScope)
	set(anotherVar "Oh")
	printVars()
endfunction()
macroScope()
printVars()

clearVars()
message("${VAR}")
set(VAR "Hi")
message("${VAR}")
