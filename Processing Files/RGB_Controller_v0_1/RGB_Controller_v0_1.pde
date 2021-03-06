import http.requests.*;

PImage img;	
String server = "http://192.168.0.152/";
String myRequest = "";

void setup() {

  //set these to the size of the image
  //size(512, 512);
  size(409, 291);

  //this is the name of your image file saved in the data folder in your
  //processing folder see processing.org for help

  //img = loadImage("RGBR.png");
  img = loadImage("RGBR2.png");

  //the [0] may be [another number] on your computer
  //myPort = new Serial(this, Serial.list()[0], 9600);
}

void draw() {
  background(0);
  image(img, 0, 0);
  img.loadPixels();
}

void mouseReleased() 
{
  
  myRequest = server;
  myRequest = myRequest + "R" + str(round((red(img.pixels[mouseX+mouseY*img.width])))*4);
  myRequest = myRequest + "G" + str(round((green(img.pixels[mouseX+mouseY*img.width])))*4); 
  myRequest = myRequest + "B" + str(round((blue(img.pixels[mouseX+mouseY*img.width])))*4);

  println(myRequest);
  //println(int((red(img.pixels[mouseX+mouseY*img.width]))));

  GetRequest get = new GetRequest(myRequest);
  get.send();
  //println("Reponse Content: " + get.getContent());
  //println("Reponse Content-Length Header: " + get.getHeader("Content-Length"));
}