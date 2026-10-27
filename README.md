# DuoFit

Jogo educativo 2D em C com Allegro 5, para Educação Física no ensino médio.
Projeto Integrador II · Senac. Equipe: Nathali, Ana Helena e Gabriel.
Entrega final: 30/11/2026.

## Executar

Com Allegro 5, compilador C e `pkg-config` instalados, execute na pasta do projeto:

```sh
./run.sh
```

O script compila a aplicacao com uma unica entrada em `main.c`.

| Arquivo | Responsabilidade |
| --- | --- |
| `main.c` | Inicializa Allegro e chama o menu e as modalidades |
| `menu.c` / `menu.h` | Menu principal |
| `handebol.c` | Fase de handebol |
| `atletismo.c` | Fase de atletismo |
| `basquete.c` | Fase de basquete |
| `fases.h` | Interface de entrada das modalidades |
| `ana_sprites.c` / `rico_sprites.c` e seus `.h` | Carregamento e recortes das imagens |
| `navegacao.c` / `navegacao.h` | Botao e teclas para voltar ao menu |
| `assets/` | Imagens do jogo |
| `prototipos/handebol.c` | Prototipo anterior, fora da compilacao principal |

No menu, use **1**, **2** ou **3** para escolher a modalidade.
Para abrir diretamente o basquete, execute `sh run-basquete.sh`
ou `sh run.sh --basquete`. **Esc** retorna ao menu.
A configuração atual de fontes usa Arial do macOS.

## Fase 2: Atletismo — Sprint 1

Responsável: Ana Helena Faustino Teodoro. Entrega prevista: 09/10/2026.
No menu, pressione **2** para entrar diretamente no atletismo.

- **Segure D ou →:** acelera para a frente até a velocidade máxima.
- **Solte:** desacelera até parar.
- **Espaço:** salta as barreiras (apenas quando está no chão).
- **E:** passa o bastão para Rico dentro da zona amarela, no chão.
- **R:** reinicia o percurso e o cronômetro.
- **Esc ou Backspace:** volta ao menu.

A Ana percorre uma pista de 100 metros com raias, marcações e linha de chegada.
A câmera acompanha a personagem e para perto do fim da pista.
O painel mostra distância, velocidade, tempo e progresso.
O cronômetro começa no primeiro movimento e para na chegada.

### Valores iniciais para ajustar no planejamento

A mecânica, o cenário e o loop da fase ficam em `atletismo.c`.
`atletismo.h` mantém apenas a declaração da função de entrada.
As constantes no início do arquivo controlam a corrida:

| Parâmetro | Valor |
| --- | --- |
| Escala | 40 pixels = 1 metro |
| Percurso | 4.000 pixels = 100 metros |
| Velocidade máxima | 520 pixels/s = 13 m/s |
| Aceleração | 260 pixels/s² = 6,5 m/s² |
| Desaceleração | 340 pixels/s² = 8,5 m/s² |
| Atualização | 60 vezes por segundo |

São valores de jogabilidade, para ajustar após experimentar.
A animação da Ana alterna os recortes de caminhada e corrida de
`assets/ana_sprites.png` a cada 0,12 segundo enquanto ela se move.
O atlas contém cinco poses: parada, duas de caminhada, corrida e salto.
`ana_sprites.h` define os recortes como sub-bitmaps da mesma textura.
O protótipo e o handebol também usam esse atlas; o lado esquerdo usa espelhamento.
### Revezamento com barreiras

Percurso didático que combina duas mecânicas do atletismo, com seis barreiras.
Ana corre o primeiro trecho; Rico espera na zona amarela entre 45 e 55 metros.
Pressione **E** dentro da zona para assumir o Rico. Se não passar o bastão,
Ana espera no fim da zona. O segundo trecho termina na linha de chegada.
Bater em uma barreira a derruba, reduz a velocidade e conta uma batida.
Salte antes da barreira para passar por cima. **R** restaura os dois trechos
 e todas as barreiras. O percurso é uma adaptação para o jogo, não uma
reprodução de uma prova oficial combinando revezamento e barreiras.

Rico usa `assets/rico_sprites.png`: parado, caminhada e salto na linha superior,
e quatro frames de corrida na linha inferior. `rico_sprites.h` cria sub-bitmaps
que compartilham a mesma textura. A corrida mantém recortes individuais para
preservar os tênis e alterna os frames pelo timer. O handebol e o basquete também
usam esse atlas; olhar para a esquerda usa `ALLEGRO_FLIP_HORIZONTAL`.
Ana alterna duas poses de corrida.

### Conferência manual

1. Entre com **2** e confirme que Ana começa parada.
2. Segure **D**: confira o aumento da velocidade e a rolagem da pista.
3. Solte: confira que Ana desacelera e para.
4. Segure **→** até a chegada: confira a mensagem e o tempo congelado.
5. Pressione **R**: posição, câmera, velocidade e tempo devem voltar ao início.
6. Volte ao menu e selecione **HANDEBOL** com W/S e **Enter**.

### Memoria das imagens

Os atlas foram reduzidos para a escala de exibicao: Ana 768×768,
Rico 768×565 e bola 128×128. Em RGBA, somam aproximadamente 4,16 MB
(antes: 71,8 MB). Os recortes compartilham a textura pai; nao sao copias.
A RAM total inclui Allegro, fontes, janela e driver grafico.
Em uma medicao local de 180 quadros do atletismo com Rico correndo,
o pico RSS foi 98,57 MB. Isso nao certifica o teto de 100 MB em todas
as fases, transicoes ou plataformas; a margem ainda e pequena.
