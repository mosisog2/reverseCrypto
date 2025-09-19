**Team members:** Baker Hasan, Hannah Dawn Aerni, Shreya Thothathri, Manha Jawahar, Andrew Kim
## Overview
Our project is on reverse software engineering and how we can apply this concept to certain cryptosystems. Reverse software engineering is the process of analyzing a program or system without source code to uncover how it works, often by examining binaries or hardware. For our proposal, we looked into reverse software engineering as a whole and also into specific cases where it was used to break cryptographic algorithms. Through this research, we figured out how we want to proceed with the report and code by drawing from recent examples. The papers we reviewed inspired us to look at how cryptographic algorithms can become vulnerable if enough internal values or operations are revealed, similar to how the ElGamal system can be compromised when critical variables are exposed even if the secret key itself is not. We plan on meeting up once a week or every two weeks to complete the project, using Teams as a way to keep in contact. *(add more here)*

## Retriever: A view-based approach to reverse engineering software architecture models

[Link](sciencedirect.com/science/article/pii/S0164121224003212)

The paper delves into reverse engineering techniques for understanding software architecture, specifically focusing on Component-Based Software Engineering (CBSE) and Unified Modeling Language (UML). The authors introduce key terms that lay the foundation for their proposed algorithm, such as Metamodel, Model, ViewType, and others. These terms are crucial for the paper's framework and methodology.

The paper proposes a view-based approach to reverse-engineering software models, emphasizing the need for models that capture the software's structure and behavior. It introduces an architecture-driven process that allows extracting and refining software models using several rules: ExtractionRule, Refinement Rule, and Finalization Rule. These rules are used in conjunction with various elements such as TraceLink, Artifact, and ModelElement to ensure that the reverse-engineered models are consistent and reflect the software's true structure.

To assess the effectiveness of their approach, the authors define a metric for evaluating reverse-engineering models. This metric is applied to various case studies, illustrating how their method improves the accuracy and completeness of the generated models compared to traditional reverse-engineering techniques.

<img width="1059" height="555" alt="image" src="https://github.com/user-attachments/assets/ffb0e983-567e-47f6-afbe-e7bcbd62443d" />
<img width="1073" height="401" alt="image" src="https://github.com/user-attachments/assets/9428e558-7161-4d5d-919a-ef8a3160e922" />
<img width="728" height="370" alt="image" src="https://github.com/user-attachments/assets/1660c9da-84c1-4f8a-ad43-63028a23d055" />
<img width="679" height="334" alt="image" src="https://github.com/user-attachments/assets/b873cbdc-c69e-43d8-bfbb-1aac5aec72ed" />
<img width="703" height="643" alt="image" src="https://github.com/user-attachments/assets/c832f65f-b573-4089-88c8-355a8725447c" />

There are several pictures to be inserted, I would recommend looking into the paper as it goes into details explaining the terminology and the algorithm.

In conclusion, the approach offers a structured methodology for reverse engineering that could potentially be applied to enhance software maintenance, improve system understanding, and facilitate architectural analysis in complex systems.

## (Reverse-Engineering a Cryptographic RFID Tag)

[Link](https://www.usenix.org/legacy/event/sec08/tech/full_papers/nohl/nohl.pdf) <br>
This paper shows how reverse engineering can be used in “man-at-the-end” attacks to dynamically extract cryptographic keys from binaries. The authors explain that keys can be located by tracing how they are used at runtime, even when they are hidden or spread across memory. They emphasize that this type of attack works because implementations often expose keys in practice, even when the cryptographic algorithm itself is secure. This gives us a clear example of how reverse engineering can directly compromise cryptographic systems, and it provides inspiration for us to design a simple demo where a secret embedded in a program can be uncovered.

## (REPQC: Reverse Engineering and Backdooring Hardware Accelerators for Post-quantum Cryptography)

[Link](https://arxiv.org/pdf/2403.09352) <br>
This paper discusses how reverse engineering can be applied to post-quantum hardware accelerators and introduces REPQC, a reverse-engineering algorithm that identifies cryptographic components such as hashing modules. Once these elements are revealed, the hardware can be backdoored, showing that implementation details pose serious risks even when algorithms are designed to be quantum-resistant. The authors argue that this makes hardware-based post-quantum systems an attractive target, and they highlight the need for defenses beyond mathematical security. This provides us with a useful reference for our report by showing that reverse engineering creates vulnerabilities not only in software but also in hardware.

## ReFormat: Automatic Reverse Engineering of Encrypted Messages

[Link](https://link.springer.com/chapter/10.1007/978-3-642-04444-1_13)

This article dives into protocol reverse engineering and how systems have been developed to automatically locate message formatting. The researchers created ReFormat, which specifically identifies decrypted messages, while other systems figure out how the message is being handled. The authors specifically work with four network protocols: HTTPS, IRC, MIME, and another unknown real-world malware. 

The goal of using ReFormat is to fill in previous gaps that other systems could not recover plaintext from ciphertext messages. It records how an input message is being processed, collects an execution trace, and then uses knowledge on how applications respond to search for the transition point between the decrypting phase and normal protocol phase. It analyzes the percentage of arithmetic and bitwise operations to find out where this transition point is, and then specifically pinpoint areas in the memory buffer that stored the decrypted message. 

The paper goes into depth on how ReFormat was able to identify the decrypted messages for each of the protocols above. It also explains how it performed these experiments depending on the protocol and what was executed in particular to achieve this goal. Although the paper was written in 2009, it applies the ideas of reverse engineering into practice and gives a good example on how it can be used to find decrypted messages from sent commands. 
## (Name of Journal/Source)
Put summary here

*Closing remarks/conclusion?*
