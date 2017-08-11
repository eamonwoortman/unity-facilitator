# unity-facilitator
A slightly modified version of the [NobleWhale's Facilitator](https://github.com/noblewhale/RakNetZebStyle/blob/master/Samples/Facilitator/).

Changes
--
- Added `-h` and `-b` parameters for setting the primary and secondary host addresses.
- Extended the service file functionality a bit

Usage
-- 

You can run the facilitator without any arguments, this will use the first ethernet adapter as the main listen address and it try to find another other available IP for external binding.
```
./Facilitator
```

If you want to be more in control as to which IP will be used, you can use the `-h` and `-b` parameters like so:

```
./Facilitator -h 86.91.40.13 -b 86.91.100.51
```

Accepted parameters are:
```
        -p      Listen port (1-65535)
        -d      Daemon mode, run in the background
        -l      Use given log file
        -e      Debug level (0=OnlyErrors, 1=Warnings, 2=Informational(default), 2=FullDebug)
        -c      Max connection count. [1000]
        -s      Statistics print delay (in seconds). [0]
        -h      Bind to listen address
        -b      Bind a second (external) addresses. When using 2 IP addresses, NATPunchthroughServer supports port stride detection, improving success rate
```

Note
--
If you are using AWS, make sure you add all 'allow all UDP traffic' to the security group.

Credits
--
Credits go to [@noblewhale](https://github.com/noblewhale) for creating his version of the Facilitator. 
