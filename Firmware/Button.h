#include <functional>
#include <string>

// Déclaration de la fonction avant la classe Bouton
std::vector<String> splitTextIntoLines(const String& text, int16_t maxWidth);
const int16_t ScreenWidth = 800;
const int16_t buttonWidth = 150;
const int16_t buttonHeight = 150;
const int16_t buttonMargin = (ScreenWidth-4*buttonWidth)/5;

class Bouton {
public:
    std::string title;
    std::string subTitle;
    bool etat; // true pour actif, false pour inactif
    std::function<void()> action; // un objet fonctionnel pour stocker l'action du bouton
    int16_t x0;
    int16_t y0;
    unsigned long startTime;

public:
    // Constructeur pour initialiser un bouton
    Bouton(const std::string &title,const std::string &subTitle, bool etat, std::function<void()> action,int16_t x0,int16_t y0)
        : title(title),subTitle(subTitle), etat(etat), action(action),x0(x0),y0(y0) {}

    // Méthode pour déclencher l'action du bouton
    void press() {
        if (etat) {
            action(); // execute l'action si le bouton est actif
        }
    }

    // ... autres méthodes ...
    void drawButton() {
        
        // Convertissez std::string en String pour l'affichage
        String titleToDisplay = String(title.c_str());
        String subTitleToDisplay = String(subTitle.c_str());
        

	const int16_t buttonRadius = 10; // Choisissez un rayon approprié pour les coins arrondis
	const uint16_t buttonColor = BLACK; // Choisissez une couleur appropriée
	
	// Obtenez les dimensions du texte
	int16_t x1, y1;
	uint16_t w, h;
	display.setTextSize(2);
	display.getTextBounds(titleToDisplay, 0, 0, &x1, &y1, &w, &h);

	// Calculer la position x pour centrer le texte
	int16_t textX = x0 + (buttonWidth - w )/ 2;

	// Calculer la position y pour centrer verticalement le texte dans le bouton
	int16_t textY = y0 + 0.4*(buttonHeight);

  if(etat)
  {
    display.fillRoundRect(x0, y0, buttonWidth, buttonHeight, buttonRadius, buttonColor);
    display.setTextColor(WHITE);
  }
  else
  {
	display.drawRoundRect(x0, y0, buttonWidth, buttonHeight, buttonRadius, buttonColor);
  }
  // Vérifiez si le titre doit être divisé en plusieurs lignes
  if (w > buttonWidth) {
        Serial.println("The button is to small !");
        // Divisez le titre en plusieurs lignes
        // Cette fonction doit être implémentée pour diviser le titre en fonction de la largeur du bouton
        std::vector<String> lines = splitTextIntoLines(titleToDisplay, buttonWidth);
        for (size_t i = 0; i < lines.size(); ++i) {
            display.getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);
            int16_t lineX = x0 + (buttonWidth - w) / 2;
            int16_t lineY = textY - h + (h * i);
            display.setCursor(lineX, lineY);
            display.print(lines[i]);
        }
    }
	else{
	  display.setCursor(textX, textY - h ); 
	  display.print(titleToDisplay);
	}
 
	
	display.getTextBounds(subTitleToDisplay, 0, 0, &x1, &y1, &w, &h);

	// Calculer la position x pour centrer le texte
	textX = x0 + (buttonWidth - w )/ 2;
	
	display.setCursor(textX, int(y0+0.9*buttonHeight - h)); 
	display.print(subTitleToDisplay);
	 display.setTextColor(BLACK);
	}
};

std::vector<String> splitTextIntoLines(const String& text, int16_t maxWidth) {
    std::vector<String> lines;
    String currentLine;
    int16_t x1, y1;
    uint16_t w, h;
    int wordStart = 0;

    for (int i = 0; i <= text.length(); ++i) {
        // Vérifiez si nous sommes à la fin d'un mot ou à la fin du texte
        if (text[i] == ' ' || i == text.length()) {
            String word = text.substring(wordStart, i);
            String testLine = currentLine + (currentLine.length() > 0 ? " " : "") + word;

            // Obtenez la largeur du texte de la ligne testée
            display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);

            if (w <= maxWidth) {
                // Ajoutez le mot à la ligne actuelle
                currentLine = testLine;
            } else {
                // Le mot ne rentre pas, enregistrez la ligne actuelle et commencez une nouvelle ligne
                if (currentLine.length() > 0) {
                    lines.push_back(currentLine);
                    Serial.println(currentLine);
                }
                currentLine = word;
            }

            wordStart = i + 1; // Commencez le prochain mot après l'espace
        }
    }

    // Ajoutez la dernière ligne
    if (currentLine.length() > 0) {
        lines.push_back(currentLine);
        Serial.println(currentLine);
    }

    return lines;
}

// Fonction qui exécutera une action nécessitant un project ID
void callApiOdoo(int projectId) {
    // Code pour faire un call à une API Odoo avec un projectId
}
