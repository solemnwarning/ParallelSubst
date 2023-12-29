# ParallelSubst

This project demonstrates an apparent bug in the Windows subst command when creating virtual drive to directory mapppings.

When `subst X: <some directory path>` is run by multiple scripts in parallel, using the same drive letter but different target directories, you would expect one to succeed and the others to fail, since only one mapping can exist for a given drive letter at a time.

What actually happens, is that depending on the timing of the commands, the subsequent subst command may return success, even though it lost the race and the drive mappings conflicted, or even incorrectly fail, if a mapping on that letter RECENTLY existed.

An example output from ParallelSubst.exe running on Windows 10:

```
Ran 4 instances of subst.exe in parallel 100 times
In 26 iterations, 0 of the subst processes succeeded
In 50 iterations, 1 of the subst processes succeeded
In 21 iterations, 2 of the subst processes succeeded
In 3 iterations, 3 of the subst processes succeeded
In 0 iterations, 4 of the subst processes succeeded
```
