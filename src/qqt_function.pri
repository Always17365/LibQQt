#------------------------------------------------------------------
#qqt_function.pri
#please dont modify this pri
#2017年12月01日14:05:43
#------------------------------------------------------------------
contains(QMAKE_HOST.os,Windows) {
    SCRIPT_SUFFIX=bat
    CMD_SEP = &
    MOVE = move /y
    COPY = copy /y
    COPY_DIR = xcopy /s /q /y /i /r /h
    MK_DIR = mkdir
    RM = del
    CD = cd /d
    RM_DIR = rd /s /q
    PATH_SEP=\\
} else {
    SCRIPT_SUFFIX=sh
    CMD_SEP = &&
    MOVE = mv
    COPY = cp -f
    COPY_DIR = cp -rf
    MK_DIR = mkdir -p
    RM = rm -f
    CD = cd
    LN = ln -sf
    RM_DIR = rm -rf
    PATH_SEP=/
}
################################################
##get command string
################################################
defineReplace(get_mkdir) {
    filepath = $$1
    isEmpty(1): error("get_mkdir(filepath) requires one argument")
    command = $${MKDIR} $${filepath}
    return ($$command)
}
defineReplace(get_errcode) {
    command = $$1
    isEmpty(1): error("get_errcode(command) requires one argument")
    contains(QMAKE_HOST.os,Windows) {
        command = $${command} >nul & echo %errorlevel%
    } else {
        command = $${command} >/dev/null; echo $?
    }
    return ($$command)
}
defineReplace(get_empty_file) {
    filename = $$1
    isEmpty(1): error("get_empty_file(filename) requires one argument")
    command = echo . 2> $${filename}
    return ($$command)
}
defineReplace(get_write_file) {
    filename = $$1
    variable = $$2
    !isEmpty(3): error("get_write_file(name, [content]) requires one or two arguments.")
    isEmpty(2) {
        return ( $$get_empty_file($$filename) )
    }
    command = echo $$variable >> $$filename
    return ($$command)
}

LINUX_CP_FILES = $${PWD}/linux_cp_files.sh
defineReplace(get_copy_dir_and_file) {
    source = $$1
    pattern = $$2
    target = $$3
    !isEmpty(4): error("get_copy_dir_and_file(source, pattern, target) requires three arguments.")
    isEmpty(3) : error("get_copy_dir_and_file(source, pattern, target) requires three arguments.")
    command = chmod +x $${LINUX_CP_FILES} $${CMD_SEP}
    command += $${LINUX_CP_FILES} $${source} $${pattern} $${target}
    return ($$command)
}

WIN_READ_INI = $${PWD}/win_read_ini.bat
LINUX_READ_INI = $${PWD}/linux_read_ini.sh
defineReplace(get_read_ini_command) {
    file_name = $$1
    sect_name = $$2
    key_name = $$3
    !isEmpty(4): error("get_read_ini_command(file, section, key) requires three arguments.")
    isEmpty(3) : error("get_read_ini_command(file, section, key) requires three arguments.")
    command =
    win32{
        #if use $${PWD}/...directoly this PWD is the refrence pri file path
        command = $${WIN_READ_INI} %file_name% %sect_name% %key_name%
    } else {
        command = $${LINUX_READ_INI} $${file_name} $${sect_name} $${key_name}
    }
    return ($$command)
}

################################################
##custom functions
################################################
# system is default a replace and test function, can be used in condition, but
# system execute succ return 1 fail return 0 it is not follow command error code.
# define test used in condition my impletement
# system_error return command errcode and it is a test function not a replace function
# 如果自定义的这个函数和系统函数重名，会调用系统函数。
defineTest(system_errcode) {
    command = $$1
    isEmpty(1): error("system_errcode(command) requires one argument")
    #special process
    command = $$get_errcode($$command)
    #the command is only return ret(0,1) wrappered by get_errcode
    ret = $$system("$${command}")
    #if eval configed ...
    #error: if(ret) : return (false)
    #erro : eval(ret = 0): return (false)
    #succ: equals(ret, 0):return (false)
    return ($$ret)
}

defineTest(mkdir) {
    filename = $$1
    isEmpty(1): error("mkdir(name) requires one argument")
    command = $$get_mkdir($$filename)
    system_errcode($${command}): return (true)
    return (false)
}

#can be used in condition or values
#must $$ !
#return values. true is 'true', false is 'false', xx0, xx1 is list
defineReplace(mkdir) {
    filename = $$1
    isEmpty(1): error("mkdir(name) requires one argument")
    command = $$get_mkdir($$filename)
    result = $$system($${command})
    return ($$result)
}


#only use in condition! return true is 1, false is 0
#refuse $$ !
#return only true(1) or false(0)
defineTest(empty_file) {
    filename = $$1
    isEmpty(1): error("empty_file(filename) requires one argument")
    command = $$get_empty_file($$filename)
    system_errcode($${command}) : return (true)
    return(false)
}

## but system write_file where ?
defineTest(write_file) {
    filename = $$1
    variable = $$2
    !isEmpty(3): error("write_file(name, [content]) requires one or two arguments.")
    isEmpty(2) {
        empty_file($$filename)
    }
    command = $$get_write_file($$filename, $$variable)
    system_errcode($$command) : return(true)
    return (false)
}

defineTest(copy_dir_and_file) {
    source = $$1
    pattern = $$2
    target = $$3
    !isEmpty(4): error("copy_dir_and_file(source, pattern, target) requires three arguments.")
    isEmpty(3) : error("copy_dir_and_file(source, pattern, target) requires three arguments.")

    command = $$get_copy_dir_and_file($$filename)
    system_errcode($${command}): return (true)
    return (false)
}
defineReplace(read_ini) {
    file_name = $$1
    sect_name = $$2
    key_name = $$3
    !isEmpty(4): error("read_ini(file, section, key) requires three arguments.")
    isEmpty(3) : error("read_ini(file, section, key) requires three arguments.")
    command = $$get_read_ini_command($$file_name, $$sect_name, $$key_name)
    echo = $$system("$$command")
    return ($$echo)
}