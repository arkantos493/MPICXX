set(IN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../doc/html_style/customsearch.js)
set(OUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../doc/html/search/search.js)

file(READ ${IN_FILE} CONTENTS)
file(APPEND ${OUT_FILE} "${CONTENTS}")