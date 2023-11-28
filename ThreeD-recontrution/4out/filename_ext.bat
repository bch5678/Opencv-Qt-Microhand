@echo off
rem 获取当前文件夹的路径
set "CurrentDir=%~dp0"
rem 去掉路径末尾的反斜杠
set "CurrentDir=%CurrentDir:~0,-1%"
rem 设置输出文件名为filelist.csv，如果已存在则删除
set "OutputFile=filelist.csv"
if exist "%OutputFile%" del "%OutputFile%"
rem 写入csv文件的第一行，即列名
echo FileName,FilePath > "%OutputFile%"
rem 遍历当前文件夹下的所有文件，写入csv文件中，用逗号分隔
for %%I in ("%CurrentDir%\*") do (
  echo %%~nxI,%%~fI >> "%OutputFile%"
)