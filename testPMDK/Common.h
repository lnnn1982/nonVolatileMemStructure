#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <iomanip> 
#include <libpmemobj.h>


/*
 * fail -- helper function to exit the application in the event of an error
 */
static void __attribute__((noreturn)) /* this function terminates */
fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}





