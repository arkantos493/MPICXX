set(IN_FILE ${CMAKE_CURRENT_LIST_DIR}/customsearch.js)
set(OUT_FILE ${CMAKE_CURRENT_LIST_DIR}/../html/search/search.js)

file(READ ${IN_FILE} CONTENTS)
file(APPEND ${OUT_FILE} "${CONTENTS}")