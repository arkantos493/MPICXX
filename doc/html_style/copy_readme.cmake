set(IN_FILE ${CMAKE_CURRENT_LIST_DIR}/../../README.md)
set(OUT_FILE ${CMAKE_CURRENT_LIST_DIR}/../main_page.md)

file(READ ${IN_FILE} CONTENTS)

string(REGEX REPLACE "\\[?![^\n]*" "" CONTENTS "${CONTENTS}")
string(REGEX REPLACE "\n\n+" "\n\n" CONTENTS "${CONTENTS}")

file(WRITE ${OUT_FILE} "${CONTENTS}")