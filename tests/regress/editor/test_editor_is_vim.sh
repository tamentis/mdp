#!/bin/sh

. ../_functions.sh


announce "editor.c:editor_is_vim(\"vim\")"
if ./stub editor_is_vim vim; then
	pass
else
	fail
fi


announce "editor.c:editor_is_vim(\"vims\")"
if ./stub editor_is_vim vims; then
	fail
else
	pass
fi


announce "editor.c:editor_is_vim(\"vim -c\")"
if ./stub editor_is_vim "vim -c"; then
	pass
else
	fail
fi


announce "editor.c:editor_is_vim(\"/usr/bin/vim\")"
if ./stub editor_is_vim /usr/bin/vim; then
	pass
else
	fail
fi


announce "editor.c:editor_is_vim(\"/usr/bin/Uim -u\")"
if ./stub editor_is_vim "/usr/bin/Uim -u"; then
	fail
else
	pass
fi


exit 0
