/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsTask.h
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	task top interface
 *
 * \date	5/09/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/


#ifndef _WELS_TASK_H_
#define _WELS_TASK_H_


namespace WelsUtilLib {


class IWelsTask
{
public:
    virtual ~IWelsTask() { }

    virtual WelsErrorType  Execute() = 0;
};

}



#endif


