#pragma once

#include <cstdlib>

const char original_url[] = "discovery.olv.nintendo.net/v1/endpoint";
const char new_url[] =      "innoverse.alwaysdata.net/olv/portal/";

_Static_assert(sizeof(original_url) > sizeof(new_url),
               "new_url too long! Must be less than 38chars.");

bool setup_olv_libs();
