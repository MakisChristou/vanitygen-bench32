## Vanitybech
Vanitybech is a tool that creates Bitcoin Segwit addresses that start with a specific pattern. For example using the patern "test" the following address and it's corresponding private key can be generated.
```
Private Key:   L3hQA4b58N9Ndx4VgGFrtYLNGGrpPVhMa1BtbPKVD48JUiQskXZk
Address:       bc1qtest4ryd8xz84kzu4xyt8yn06ak24dn6nm763p
```

Since Bitcoin addresses are not reversable the only way to do this is to generate as many random private keys as possible, and check if their address matches the pattern. This is done repeatedly until a matching pattern is found.

## Estimated Time
Guessing a private key with the chosen prefix has an element of luck to it. The following table depicts a rough estimate on what should  be expected on a Ryzen 3600 desktop processor using all of it's 12 threads. Your results may vary.

| Prefix      | Eta (Ryzen 3600)|
| ------------- | ---------- |
|bc1q0        	  | 20 ms           |
|bc1q00           | 20 ms           |
|bc1q000          | 150 ms          |
|bc1q0000         | 10 s            |
|bc1q00000        | 5 min           |
|bc1q000000       | 3 hours         |
|bc1q0000000      | 3 days          |
|bc1q00000000     | 100 days        |
|bc1q000000000    | 10 years        |
|bc1q0000000000   | 350 years       |
|bc1q00000000000  | 10000 years     |


## Compilation (Ubuntu 20.10)
```
sudo apt install build-essential libgmp-dev libssl-dev autoconf libtool
make
```

## Example Usage
```
$ ./vanitygen -p bc1q0000
Pattern: bc1q0000
Generating BTC Address
[00:00:00:00][7 Kkey/s][Total 1][Eta 3 min]
[00:00:00:05][23 Kkey/s][Total 103638][Eta 51 sec]
[00:00:00:10][20 Kkey/s][Total 205645][Eta 57 sec]
[00:00:00:15][14 Kkey/s][Total 301861][Eta 81 sec]
[00:00:00:20][21 Kkey/s][Total 397760][Eta 55 sec]
[00:00:00:25][21 Kkey/s][Total 497344][Eta 56 sec]
[00:00:00:30][21 Kkey/s][Total 598619][Eta 55 sec]
[00:00:00:35][21 Kkey/s][Total 698003][Eta 56 sec]
[00:00:00:40][21 Kkey/s][Total 794843][Eta 56 sec]

Private Key:   L179CEmQh13Z17LzR8MVyVMLhoSGG6FXScDtt7oVZFe1qk46eJHr
Address:       bc1q0000erwyfzl9m6y9yr8sun52p6tvmrzp5jpwtt
```

## Important Notes
When using this software double check the generated addresses using other implementations to make sure that everyting is working correctly. Do NOT send mainnet coins to generated addresses unless you make sure that the address matches the coresponding private key. I would recommend this open source [tool](https://segwitaddress.org/bech32/) for double checking. Also, please do not use the addresses shown in this readme file as you will (probably) get robbed.

## Notes for macOS
If building on macOS, please install the secp256k1 package using (home)brew, as the version for macOS is not packaged into this repository.