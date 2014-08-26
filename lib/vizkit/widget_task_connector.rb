module Vizkit
    class WidgetTaskConnector
        def initialize(widget,task,options = Hash.new)
            raise ArgumentError,"#{widget} is not a qt object" unless widget.is_a? Qt::Object
            @widget = widget
            @task = task
            @options = options
        end

        def evaluate(&block)
            @self_before_instance_eval = eval "self", block.binding
            instance_exec @task, &block
        end

        def connect(sender,receiver=nil,options= Hash.new,&block)
            source = resolve(sender)
            receiver,options= if receiver.is_a? Hash
                                  [nil,receiver]
                              else
                                  [receiver,options]
                              end
            if receiver && block
                raise ArgumentError "Code blocks act as receivers and are not supported if a reveiver is already given."
            end
            receiver = resolve(receiver || block)

            options[:getter] = resolve(options[:getter]) if options[:getter]
            options[:callback] = resolve(options[:callback]) if options[:callback]
            source.connect_to receiver,options
        end

        def method_missing(method, *args, &block)
            @self_before_instance_eval.send method, *args, &block
        end

        def PORT(str)
            "5#{str}"
        end

        def OPERATION(str)
            "6#{str}"
        end

        def EVENT(str)
            "7#{str}"
        end

        def PROPERTY(str)
            "8#{str}"
        end

        #Wrapper method to connect sub widgets which are plain Qt ruby classes
        def wrap(widget)
            WidgetTaskConnector.new(widget,@task,@options)
        end

        def method_missing(m,*args)
            if @widget.respond_to? m
                WidgetTaskConnector.new(@widget.send(m),@task,@options)
            else
                super
            end
        end

        private
        # converts the given string into a list of object which meed the given signature
        # @return [Array(Spec)]
        def resolve(signature,options = Hash.new)
            signature =~ /^(\d)(.*)/
                case $1.to_i
                when 1
                    ConnectorSlot.new(@widget,$2,options)
                when 2
                    ConnectorSignal.new(@widget,$2,options)
                when 5
                    ConnectorPort.new(@task,$2,options)
                when 6
                    ConnectorOperation.new(@task,$2,options)
                when 7
                    ConnectorEvent.new(@task,$2,options)
                when 8
                    ConnectorProperty.new(@task,$2,options)
                else
                    if signature.is_a? Symbol
                        ConnectorSlot.new(@widget,signature.to_s,options)
                    elsif signature.respond_to? :call
                        ConnectorProc.new(@task,signature,options)
                    else
                        raise ArgumentError,"#{signature} has an invalid type identifyer #{$1}"
                    end
                end
        end
    end
end

