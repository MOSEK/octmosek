#ifndef OMSK_MSG_MOSEK_H_
#define OMSK_MSG_MOSEK_H_

#include "mosek.h"
#include "omsk_msg_base.h"

#include <string>

// ------------------------------
// RESPONSE AND EXCEPTION SYSTEM
// ------------------------------

void errcatch(MSKrescodee r, std::string str);
void errcatch(MSKrescodee r);

msk_response get_msk_response(MSKrescodee r);

#endif /* OMSK_MSG_MOSEK_H_ */
