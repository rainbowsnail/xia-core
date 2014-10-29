﻿package edu.cmu.networkelements {	import flash.display.MovieClip;	import flash.text.*;	import flash.events.*;	import flash.geom.Point;	import flash.utils.Dictionary;	import edu.cmu.MyGlobal;		public class NetworkElement extends MovieClip {				// Constants:		// Public Properties:		// Private Properties:		protected var elementID:String;  // Unique (probably an HID)		protected var elementName:String;  // Something readable like "Router 1" or "David's Laptop" (for display only)		protected var ports:Array;  // holds connection objects		protected var connectedElements:Dictionary; // maps elements to port numbers		protected var displayHID:Boolean;				// UI Elements:		protected var nameLabel:TextField		protected var idLabel:TextField;		protected var widthWithoutLabel:int;			// Initialization:		public function NetworkElement(elementID:String, elementName:String, numPorts:int) {			this.elementID = elementID;			this.elementName = elementName;			ports = new Array(numPorts);			connectedElements = new Dictionary();			displayHID = false;						this.addEventListener(Event.RENDER, render_handler);			this.addEventListener(MouseEvent.MOUSE_DOWN, mouse_press);			this.addEventListener(MouseEvent.MOUSE_UP, mouse_release);			this.doubleClickEnabled = true;			this.addEventListener(MouseEvent.DOUBLE_CLICK, mouse_click);						configUI();		}			// Public Methods:		public function GetElementName():String {			return elementName;		}				public function GetElementID():String {			return elementID;		}				public function GetElementIDAtPort(port:int):String {			if (ports[port] == null) {				return null;			} else {				return ports[port].GetRemoteElementID(this);			}		}				public function GetLocalPortForElement(element:NetworkElement):int {			return connectedElements[element];		}				public function GetRemotePortForElement(element:NetworkElement):int {						return ports[connectedElements[element]].GetRemoteElementPort(this);		}				public virtual function Copy():NetworkElement {			trace("ERROR: NetworkElement:Copy() should never execute");			return null;		}				public function GetCenterPoint():Point {			var centX:int = this.x + (widthWithoutLabel / 2);			var centY:int = this.y + (this.height / 2);			//trace(elementID + "  X=" + this.x + " Y=" + this.y + "  W=" + this.width + " H=" + this.height + "  CX=" + centX + " CY=" + centY);			return new Point(centX, centY);		}				public function GetEdgePointNearestPoint(point:Point):Point {			// TODO: make this more sophisticated			var myCenter:Point = this.GetCenterPoint();			var myRadius:Number = Math.sqrt(Math.pow(widthWithoutLabel/2, 2) + Math.pow(this.height/2, 2));  // +10 for id label			var distance:Number = Point.distance(myCenter, point);			return Point.interpolate(myCenter, point, 1-(myRadius/distance)); //(30/distance));					}		//		public function SendPacket(destination:NetworkElement, type:String) {//			// TODO: error checking//			//			// Get the appropriate connection//			var connection:Connection = ports[connectedElements[destination]];//			//			// Tell the connection to send a pacekt//			connection.SendPacket(this, type);//		}				// Make a connection between this element and a remote one.		// If this element already has a connection to a different element		// on the local port, disconnect it first		public function ConnectElementToPort(element:NetworkElement, localPort:int, remotePort:int):Connection {			// TODO: error checking			//trace("connecting " + elementID + "[" + localPort + "] to [" + remotePort + "]" + element.GetElementID());			var connection:Connection;						// Check to see if a connection already exists on localPort			if (ports[localPort] != null)			{				if (ports[localPort].GetRemoteElementID(this) == element.GetElementID()) {					// We're already connected to the right element; just return the connection					return ports[localPort];				}				else				{					DisconnectElementAtPort(localPort);				}			}						connection = new Connection(this, localPort, element, remotePort);			ports[localPort] = connection;			connectedElements[element] = localPort;			element.AddConnectionToPort(connection, this, remotePort);						return connection;		}				// Removes connection and tells the element at the other end		// to do the same. Also removes connection object from stage		public function DisconnectElementAtPort(port:int):void {			if (port < 0 || port >= ports.length) { return; }						if (ports[port] != null) {				ports[port].RemoveConnection();  // this call tells connection to call RemoveConnectionFromPort on each parent element			}		}				public function DisconnectAllPorts():void {			for (var i:int = 0; i < ports.length; i++) {				DisconnectElementAtPort(i);			}		}				public function AddConnectionToPort(connection:Connection, remoteElement:NetworkElement, port:int):void {						if (port == -1) {				port = GetUnusedPort();			}						if (ports[port] != null)  // TODO: test this?			{				if (ports[port].GetRemoteElementID(this) != remoteElement.GetElementID())				{					DisconnectElementAtPort(port);				}				else				{					return;				}			}						ports[port] = connection;			connectedElements[remoteElement] = port;		}				// Simply removes connection object from port array. (needed this		// separate method to avoid an infinite loop)		public function RemoveConnectionFromPort(port:int):void {			ports[port] = null;		}				// Given a "model" element and a dictionary of network elements		// to connect to, make this element's connections match the model's		public function UpdateConnections(model:NetworkElement, elementsByID:Dictionary, newDict:Dictionary) {			for (var i:int = 0; i < ports.length; i++) 			{				if (model.GetElementIDAtPort(i) == null) {					this.DisconnectElementAtPort(i);				} else {					var oldRemoteElement:NetworkElement = elementsByID[model.GetElementIDAtPort(i)];					var newRemoteElement:NetworkElement = newDict[model.GetElementIDAtPort(i)];					this.ConnectElementToPort(oldRemoteElement, i, model.GetRemotePortForElement(newRemoteElement));				}			}		}				// currentTraffic maps type:String to pkts/sec:int		public function UpdateOutboundTrafficForPort(port:int, currentTraffic:Dictionary) {			if(ports[port] != null) {				ports[port].UpdateTraffic(this, currentTraffic);			}		}				// moves this element to the top of the stage so that the		// picture and name label are on top of the link		public function moveToTop():void {			this.parent.setChildIndex(this, this.parent.numChildren - 1);		}				// Protected Methods:		protected function configUI():void {			widthWithoutLabel = this.width;						nameLabel = MyGlobal.makeLabel(elementName, 0x000000, 13, true);			nameLabel.x = (this.width - nameLabel.width) / 2;			nameLabel.y = this.height;			this.addChild(nameLabel);						idLabel = MyGlobal.makeLabel(elementID, 0x666666, 11);			idLabel.x = (widthWithoutLabel - idLabel.width) / 2;			idLabel.y = this.height - 3;			if (displayHID) {				this.addChild(idLabel);			}		}				protected function redrawConnections():void {			for each (var connection:Connection in ports) {				if (connection != null) {					connection.Redraw();				}			}			moveToTop();		}				protected function GetUnusedPort():int {			for (var i:int = 0; i < ports.length; i++) {				if (ports[i] == null) {					return i;				}			}			trace("Could not find unused port on element " + elementID);			return -1;		}				protected function render_handler(e:Event) {						this.redrawConnections();		}				protected function mouse_press(e:MouseEvent) {			this.startDrag();		}				protected function mouse_release(e:MouseEvent) {			this.stopDrag();			this.redrawConnections();			trace(this.x + ", " + this.y);		}				protected function mouse_click(e:MouseEvent) {			displayHID = !displayHID;						if (displayHID && !this.contains(idLabel)) {				this.addChild(idLabel);			}			if (!displayHID && this.contains(idLabel)) {				this.removeChild(idLabel);			}		}	}}