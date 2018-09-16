# Sistema de arquivo
Desenvolver um sistema de arquivos distribuídos que simula padrão EXT3. O sistema deve ser desenvolvido em C ou C++ utilizando o compilador GNU GCC e chamadas de sistemas do padrão POSIX. 

O sistema deve permitir que arquivos locais sejam acessados por usuários remotos simultaneamente. As operações permitidas pelo sistema devem incluir:

    criar (sub)diretório
    remover (sub)diretório
    entrar em (sub)diretório
    mostrar conteúdo do diretório
    criar arquivo 
    remover arquivo
    escrever um sequência de caracteres em um arquivo
    mostrar conteúdo do arquivo

Etapa 1

Desenvolver a estrutura de acesso do servidor de arquivos. Ele deverá será acessado via socket TCP. Cada conexão deverá ser gerida por uma thread. Condições de corrida deverão ser tratadas por meio de semáforos ou mutexes. Nesta etapa você não precisa implementar as operações sobre arquivos listadas acima. Ao invés disso, use as operações diretamente do sistema de arquivos do seu sistema operacional. Recomenda-se que o servidor imprima mensagens na tela para demonstrar o funcionamento ao professor.

Observações:

    Não é necessário autenticação dos usuários.
    Não é necessário criar um aplicativo cliente. Você pode usar o aplicativo netcat disponível para Linux e Windows.

Etapa 2

Estenda o trabalho desenvolvendo a estrutura de alocação e organização de arquivos do servidor de arquivos. Mais precisamente, agora a aplicação servidor utilizará um arquivo em disco que simulará o sistema de arquivos EXT3. Dentro deste arquivo haverá no mínimo três regiões: uma região para o controle de espaço livre, uma região para armazenar inodes e uma região para armazenar dados de arquivos e diretórios. As operações devem ser implementadas usando o arquivo que simula o sistema de arquivos. Por exemplo, assuma que o arquivo que simula o sistema de arquivos se chama sa.bin e que um usuário quer criar um arquivo chamado teste.txt no servidor contendo os caracteres "abcd" (4 bytes). Então, dentro de sa.bin o servidor deve encontrar um inode livre, atribuir o nome teste.txt ao inode e associar um bloco livre de 4 bytes para conter "abcd". Ainda dentro de sa.bin, grava-se "abcd" no bloco livre associado e se deduz espaço utilizado para guardar "abcd" do espaço livre.

A thread no servidor deve receber uma solicitação do cliente e então repassá-la a um novo processo via pipe nomeado e este deverá executar a ação solicitada e devolver ao servidor uma mensagem de status, informando se a solicitação foi realizada com sucesso ou com erro.

Informações

O trabalho deverá ser feito em dupla ou individualmente. No dia da apresentação, data que deverá ser marcada com antecedência com o professor, o aluno ou alunos irão apresentar o programa em execução mostrando as funcionalidades do mesmo, bem como apresentar o código fonte. As notas serão atribuídas de maneira individual, ou seja, caso o trabalho tenha sido feito em dupla cada um dos autores receberá uma nota. Esta nota dependerá do desempenho durante apresentação do trabalho onde o professor arguirá o aluno. O trabalho deverá ser desenvolvido na linguagem de programação C usando as bibliotecas de semáforos, threads e comunicação entre processos (socket local ou pipe nomeado). 
