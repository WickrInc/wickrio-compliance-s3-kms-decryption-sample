/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: MIT-0
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <stdio.h>
#include <sys/stat.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>

#include <aws/s3/S3Client.h>
#include <aws/s3/model/Object.h>
#include <aws/s3-encryption/S3EncryptionClient.h>
#include <aws/s3-encryption/CryptoConfiguration.h>
#include <aws/s3-encryption/materials/KMSEncryptionMaterials.h>
#include <aws/s3-encryption/materials/SimpleEncryptionMaterials.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/ListObjectsResult.h>
#include <aws/core/utils/crypto/Cipher.h>

using namespace std;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::S3Encryption;
using namespace Aws::S3Encryption::Materials;

Aws::String m_region;
Aws::String m_bucketName;
Aws::String m_folderName;
Aws::String m_masterKeyID;
Aws::String m_destFolder;

Aws::String SplitFilename (const string& str)
{
  size_t found;
  cout << "Splitting: " << str << endl;
  found=str.find_last_of("/\\");
  Aws::String folder = str.substr(0,found);
  cout << " folder: " << folder << endl;
  cout << " file: " << str.substr(found+1) << endl;
  return folder;
}

bool getEncryptedData(const Aws::String& keyName)
{
    //get an encrypted object from S3
    GetObjectRequest getRequest;
    getRequest.WithBucket(m_bucketName)
        .WithKey(keyName);

    auto kmsMaterials = Aws::MakeShared<KMSWithContextEncryptionMaterials>("s3EncryptionV2", m_masterKeyID);
    CryptoConfigurationV2 cryptoConfig(kmsMaterials);
    S3EncryptionClientV2 encryptionClient(cryptoConfig);

    auto getObjectOutcome = encryptionClient.GetObject(getRequest);
    if (getObjectOutcome.IsSuccess())
    {
        const Aws::Map<Aws::String, Aws::String> metadata = getObjectOutcome.GetResult().GetMetadata();
        auto search = metadata.find("wickrio-type");
        if (search != metadata.end()) {
            Aws::String first = search->first;
            Aws::String second = search->second;
            std::cout << "Successfully retrieved wickr-type metadta: " << second << std::endl;

            if (second == "file") {
                auto fpsearch = metadata.find("wickrio-file-path");
                Aws::String filePath = fpsearch->second;
                std::cout << "Successfully retrieved wickr-file-path metadta: " << filePath << std::endl;

                // TODO: Check if there is a slash between the destFolder and the filePath
                Aws::String filesDir = SplitFilename (filePath);
                Aws::String filename;
                if (filesDir.length() > 0) {
                    filename = m_destFolder + filePath;
                    Aws::String dirname = m_destFolder + filesDir;

                    // Check if the directory exists
                    struct stat info;
                    if (stat(dirname.c_str(), &info) == -1) {
                        // Create the full path for the file
#if 0
                        if (mkdir(dirname.c_str(), 0777) == -1) {
                            std::cout << "Could not create directory:" << dirname << std::endl;
                            return false;
                        }
#else
                        Aws::String command;
                        command = "mkdir -p " + dirname;
                        system(command.c_str());
#endif
                    }
                } else {
                    filename = m_destFolder + "/" + filePath;
                }
                std::istream input(getObjectOutcome.GetResult().GetBody().rdbuf());
                std:ofstream newFile(filename, std::ios::binary);

                std::copy(std::istreambuf_iterator<char>(input),
                          std::istreambuf_iterator<char>( ),
                          std::ostreambuf_iterator<char>(newFile));

                newFile.close();
            }
        }

        std::cout << "Successfully retrieved object from s3 with value: " << std::endl;
//        std::cout << getObjectOutcome.GetResult().GetBody().rdbuf() << std::endl << std::endl;;
        return true;
    }
    else
    {
        std::cout << "Error while getting object " << getObjectOutcome.GetError().GetExceptionName() <<
            " " << getObjectOutcome.GetError().GetMessage() << std::endl;
        return false;
    }
}


bool listObjects(Aws::Client::ClientConfiguration &clientConfig)
{
    Aws::S3::S3Client s3_client(clientConfig);
    Aws::String marker;

    do {
        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(m_bucketName);
        request.WithPrefix(m_folderName);
        if (marker.length() > 0)
            request.WithMarker(marker);

        auto outcome = s3_client.ListObjects(request);

        if (outcome.IsSuccess()) {
            std::cout << "Objects in bucket '" << m_bucketName << "':"
                << std::endl << std::endl;

            Aws::Vector<Aws::S3::Model::Object> objects =
                outcome.GetResult().GetContents();

            for (Aws::S3::Model::Object& object : objects) {
                std::cout << object.GetKey() << std::endl;

                getEncryptedData(object.GetKey());
            }
            marker = outcome.GetResult().GetMarker();
        } else {
            std::cout << "Error: ListObjects: " <<
                outcome.GetError().GetMessage() << std::endl;

            return false;
        }
    } while (marker.length() > 0);
    return true;
}

int main(int argc, char *argv[])
{

    if (argc != 6) {
        cout << "Usage: " << std::string(argv[0]) << "<region> <S3 bucketname> <S3 folder> <KMS ARN> <destination directory>" << endl;
        return 1;
    }

    m_region = argv[1];
    m_bucketName = argv[2];
    m_folderName = argv[3];
    m_masterKeyID = argv[4];
    m_destFolder = argv[5];

    /*
     * Setup AWS SDK
     */

    Aws::SDKOptions awsSdkOptions;
    awsSdkOptions.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
    Aws::InitAPI(awsSdkOptions);

    Aws::Client::ClientConfiguration    awsClientConfig;
    awsClientConfig.region = m_region;

    listObjects(awsClientConfig);

    Aws::ShutdownAPI(awsSdkOptions);
    return 0;
}
