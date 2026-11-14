# DuoFit

Jogo educativo 2D em C com Allegro 5, para Educação Física no ensino médio.
Projeto Integrador II - Senac. Equipe: Nathali, Ana Helena e Gabriel.
Entrega final: 30/11/2026.

## Arquivos

| Arquivo | O que faz |
| --- | --- |
| `main.c` | Liga o Allegro, abre a janela e troca entre o menu e as fases |
| `duofit.h` | Constantes, variáveis globais e funções que todos os arquivos usam |
| `comum.c` | Carrega fontes, imagens e som uma vez só; funções `escrever` e `desenhar` |
| `menu.c` | Menu principal |
| `handebol.c` | Fase 1 |
| `atletismo.c` | Fase 2 |
| `basquete.c` | Fase 3 |
| `pergunta.c` | Pergunta do final de cada fase e tela de pontos |
| `assets/` | Imagens (`ana_sprites.png`, `rico_sprites.png`, `bola.png`) e som (`gol.wav`) |

## Como o jogo funciona

```
MENU -> Handebol -> Atletismo -> Basquete -> MENU
```

Cada fase tem as mesmas telas:

1. **Regras** (fundo azul). ENTER começa.
2. **Jogo**.
3. **Vitória** (fundo verde). ENTER abre a pergunta.
4. **Pergunta** com 3 alternativas (A, B ou C). Acertar vale +10 pontos.
5. **Pontos finais**: pontos da fase, da pergunta e o total do DuoFit.

O jogo usa só o teclado. No menu, W/S escolhe e ENTER entra.

O jogo sempre começa pelo handebol. As outras fases aparecem em cinza
(bloqueadas), não podem ser selecionadas e são liberadas em ordem: passar do handebol libera o atletismo,
e passar do atletismo libera o basquete. O menu mostra fotos da Ana e do Rico em porta-retratos. Para usar fotos próprias,
coloque `foto_ana.png` e `foto_rico.png` na pasta `assets/` (de preferência em pé, proporção 2:3).

Cada fase é uma função com um loop que espera um evento (tecla ou timer),
atualiza o jogo 60 vezes por segundo e desenha a tela. A variável `estado`
diz qual tela está aparecendo. Quando a fase termina, a função devolve para o
`main` se é para ir para a próxima fase (`PROXIMA_FASE`), voltar ao menu
(`VOLTAR_MENU`) ou fechar o jogo (`FECHAR_JOGO`).

| Tecla | Handebol | Atletismo | Basquete |
| --- | --- | --- | --- |
| A / D | andar | D (segurar): correr | andar |
| W | pular | pular a barreira | pular |
| S | quicar | - | quicar |
| ESPAÇO | arremessar | passar o bastão | arremessar |
| ENTER | continuar | continuar | continuar |
| ESC / BACKSPACE | menu | menu | menu |

### Pontos (mínimo 10 por fase)

- **Handebol:** 30 menos 2 por arremesso que não foi gol. Meta: 3 gols em até
  7 tentativas. Se as tentativas acabarem, a fase recomeça.
- **Atletismo:** 30 pontos. Meta: chegar aos 100 m sem derrubar nenhuma barreira.
  Se derrubar uma, a fase recomeça.
- **Basquete:** 30 menos 2 por arremesso errado. Meta: 10 pontos de cesta
  (3 de trás da linha branca, 2 dentro) em até 7 tentativas. Se as tentativas
  acabarem, a fase recomeça. Mais de 2 passos sem quicar é andada.

Cada fase vale no máximo 40 (30 + 10 da pergunta), então o DuoFit vale 120.
Para **ganhar o jogo** é preciso terminar com pelo menos 80% (96 pontos).
Abaixo disso aparece a mensagem de que o jogador perdeu.

### Física

Tudo é contado por frame (60 por segundo):

- `posicao = posicao + velocidade` e `velocidade = velocidade + gravidade`,
  o que faz o pulo e a bola andarem em parábola.
- No atletismo, segurar D soma 0,07 na velocidade a cada frame (máximo 8,5 pixels
  por frame) e soltar tira 0,1. A pista tem 40 pixels por metro.
- No basquete, a bola sai inclinada uns 67 graus. A barra de força muda a velocidade
  de saída, por isso cada distância precisa de uma força diferente para acertar.

### Som

`assets/gol.wav` toca no gol, na cesta e na chegada do atletismo.
Sem o arquivo ou sem placa de som, o jogo roda normalmente, só sem som.

### Memória

Fontes, imagens e som são carregados uma vez só no começo (`carregar_recursos`)
e todas as fases usam os mesmos. As poses dos personagens são recortes
(`al_create_sub_bitmap`) da mesma imagem, então não ocupam memória a mais.

## Visual Studio (Windows)

1. Crie um projeto **C++ vazio** e adicione os 7 arquivos `.c` e o `duofit.h`
   (Adicionar > Item Existente).
2. Botão direito no projeto > **Gerenciar Pacotes NuGet** > instale **Allegro**.
3. Propriedades do projeto > **Allegro 5** > Add-ons: marque **Image, TTF, Font,
   Primitives, Audio e Audio Codec**.
4. Coloque a pasta `assets/` ao lado do arquivo `.vcxproj`.
5. **Compilação > Recompilar Solução** e **F5**.

## Linux ou macOS

Com Allegro 5 e `pkg-config` instalados, rode `./run.sh` na pasta do projeto.
