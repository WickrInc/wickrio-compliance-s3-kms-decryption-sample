# WickrIO Compliance Service S3 KMS Decryption Sample

This is sample C++ code that can be compiled and used to decrypt the files saved in an S3 bucket using the client side encryption functionality of the Wickr Pro Compliance service. The program compiles on most platforms.  You will need to have the C++ AWS SDK installed and built for S3 and KMS, instructions to build that:

```
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
mkdir build
cd build
cmake <directory where AWS SDK installed>/aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="kms;s3;s3-encryption"
```

You can use cmake to build the sample code to an executable binary.  Running the built program has the following usage:

```
./WickrIOCSEDecryption <region> <S3 bucketname> <S3 folder> <KMS ARN> <destination directory>
```

The region is the AWS region where the S3 bucket and KMS Master key is located.  The "S3 bucketname" is the name of the S3 bucket.  The "S3 folder" is the name of the folder in the S3 bucket where the files are located. The "KMS ARN" is the ARN used to encrypt the files located in the S3 bucket. The "desination directory" is the location where the decrypted files should be put.

-## Security
 
-See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

This library is licensed under the MIT-0 License. See the LICENSE file.

