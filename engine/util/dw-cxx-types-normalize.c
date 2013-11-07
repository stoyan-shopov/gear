
note: here class and struct are treated the same for cxx

(1) sweep thru .debug_info and locate all class definitions
(2) build class hash tables, hash keys are the linkage names of
	all members of all classes gathered in step (1)
(3) sweep thru the .debug_info section and for all attributes of all dies:
	(3.1) if a dwarf attribute is of reference type, and:
		- if the die referenced is a class declaration,
		then overwrite the reference value
		with a global .debug_info off-set reference to the appropriate
		class definition die; here, matching a class declaration die to a
		a class definition die can be tricky - one suggestion is to
		look for a member of the class declaration wich has a linkage
		name attribute; if no such member is found, do nothing (do not
		overwrite the reference attribute, skip the die altogether);
		if such a member is found - using the member linkage name as
		a key in the hash table built at step (2) does seem to be
		a very reliable heuristics (after all, these names are what
		the loader uses when adjusting program addresses, so these
		should be pretty accurate)
		- if the die referenced is a class member (either a function, or
		a variable), similarly to the case above make use of the
		member linkage name to modify the reference to point
		to the appropriate member in the proper class definition
	(3.2) for all of the dies which got a reference of theirs overwriten
		at step (3.1), force the corresponding .debug_abbrev attribute
		form to be of a global reference (offset from .debug_info) form 
	(3.1) for all compilation units which had a die of theirs modified in
		step (3.1), adjust their headers in .debug_info (the overall
		compilation unit size fields in the compilation	unit headers
		in .debug_info) if the size of the compilation unit in question
		has changed

