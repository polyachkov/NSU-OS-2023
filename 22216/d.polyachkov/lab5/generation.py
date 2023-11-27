f = open("generatedFile", "w")

str1 = "123456789"
str2 = "abcdefgh"

for i in range(1, 500):
	f.write(str1)
f.write("\n")
for i in range(1, 500):
	f.write(str2)
f.close()